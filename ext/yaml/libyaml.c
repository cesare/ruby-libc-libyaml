#include <ruby.h>

#ifdef RUBY_RUBY_H
# include <ruby/io.h>
#else
# include <rubyio.h>
#endif

#include <yaml.h>

#define GLOBAL_DEFINE
#include "libyaml.h"

#define NOT_IMPLEMENTED rb_raise(rb_eNotImpError, "not implemented!!");

/*
 * prototypes of internal functions and methods
 */
static VALUE rb_libyaml_load(VALUE self, VALUE rstr);
static VALUE rb_libyaml_load_file(VALUE self, VALUE file_str);
static VALUE rb_libyaml_load_stream(VALUE self, VALUE rstr);
static VALUE rb_libyaml_dump(VALUE self, VALUE robj, VALUE io);

static VALUE loader_alloc(VALUE clazz);

static FILE* get_iostream(VALUE io);

static VALUE mYAML;

static VALUE cLoader;
static VALUE cStream;

void Init_native()
{

  mYAML = rb_define_module("YAML");
  mLibYAML = rb_define_module_under(mYAML, "LibYAML");

  const char* oct_reg_char = "^0[0-7]+$";
  rb_define_const(mLibYAML, "OCT_REGEX", rb_reg_new(oct_reg_char, strlen(oct_reg_char), 0));

  const char* hex_reg_char = "^0x[0-9a-fA-F]+";
  rb_define_const(mLibYAML, "HEX_REGEX", rb_reg_new(hex_reg_char, strlen(hex_reg_char), 0));

  const char* num_reg_char = "^(\\+|-)?([0-9][0-9\\._]*)([eE][\\+-]?\\d+)?$";
  rb_define_const(mLibYAML, "NUM_REGEX", rb_reg_new(num_reg_char, strlen(num_reg_char), 0));

  rb_define_singleton_method(mLibYAML, "load", rb_libyaml_load, 1);
  rb_define_singleton_method(mLibYAML, "load_file", rb_libyaml_load_file, 1);
  rb_define_singleton_method(mLibYAML, "load_stream", rb_libyaml_load_stream, 1);
  rb_define_singleton_method(mLibYAML, "dump", rb_libyaml_dump, 1);

  cLoader = rb_define_class_under(mLibYAML, "Loader", rb_cObject);
  rb_define_alloc_func(cLoader, loader_alloc);

  cStream = rb_define_class_under(mLibYAML, "Stream", rb_cObject);
  rb_define_attr(cStream, "documents", TRUE, TRUE);
}


static VALUE loader_alloc(VALUE klass) {
  ParsingContext* context;
  yaml_parser_t* parser;
  VALUE loader;

  loader = Data_Make_Struct(klass, ParsingContext, 0, -1, context);

  parser = (yaml_parser_t*)ALLOC(yaml_parser_t);
  if (!yaml_parser_initialize(parser)) {
    rb_sys_fail("Failed to initialize parser instance with yaml_parser_initialize()");
  }
  context->parser = parser;

  return loader;
}


static VALUE destroy_context(VALUE loader) {
  ParsingContext* context;
  Data_Get_Struct(loader, ParsingContext, context);

  if (context->parser != NULL) {
    yaml_parser_delete(context->parser);
    context->parser = NULL;
  }

  if (context->source_type == STREAM) {
    if (context->source.stream.close_on_exit && context->source.stream.io != NULL) {
      context->source.stream.io = NULL;
    }
  }

  return Qtrue;
}


static VALUE load_single_document(ParsingContext* context) {
  yaml_document_t document;
  yaml_node_t* root;
  VALUE results;

  if (!yaml_parser_load(context->parser, &document)) {
    return Qundef; /* TODO raise exception */
  }

  root = yaml_document_get_root_node(&document);
  if (root == NULL) {
    return Qundef;
  }

  results = construct_node(&document, root);

  yaml_document_delete(&document);
  return results;
}

static VALUE load_documents(VALUE loader) {
  yaml_parser_t* parser;
  VALUE results;
  VALUE doc;
  VALUE documents;
  ParsingContext* context;

  Data_Get_Struct(loader, ParsingContext, context);

  parser = context->parser;
  switch (context->source_type) {
    case STRING:
      yaml_parser_set_input_string(parser, context->source.string.str, context->source.string.length);
      break;
    case STREAM:
      yaml_parser_set_input_file(parser, context->source.stream.io);
      break;
  }

  if (!context->parse_all_documents) {
    return load_single_document(context);
  }

  results = rb_ary_new();
  while (TRUE) {
    doc = load_single_document(context);
    if (doc == Qundef) {
      break;
    }
    rb_ary_push(results, doc);
  }

  documents = rb_class_new_instance(0, NULL, cStream);
  rb_funcall(documents, rb_intern("documents="), 1, results);
  return documents;
}




static VALUE rb_libyaml_load(VALUE self, VALUE rstr) {
  VALUE loader;
  ParsingContext* context;

  loader = rb_class_new_instance(0, NULL, cLoader);
  Data_Get_Struct(loader, ParsingContext, context);

  switch (TYPE(rstr)) {
    case T_STRING:
      context->source_type = STRING;
      context->source.string.str = (const unsigned char*)RSTRING_PTR(rstr);
      context->source.string.length = RSTRING_LEN(rstr);
      break;
    case T_FILE: {
      FILE* io = get_iostream(rstr);
      if (io == NULL) {
        rb_raise(rb_eArgError, "Failed to get FILE object");
      }
      context->source_type = STREAM;
      context->source.stream.io = io;
      context->source.stream.close_on_exit = FALSE;
      break;
    }

    default:
      rb_raise(rb_eTypeError, "1st argument must be String or IO instances");
  }
  return rb_ensure(load_documents, loader, destroy_context, loader);
}


static VALUE rb_libyaml_load_file(VALUE self, VALUE filename) {
  ParsingContext* context;
  VALUE loader;
  const char* name;
  FILE* file;

  /* parameter filename must be a string object */
  Check_Type(filename, T_STRING);

  name = RSTRING_PTR(filename);
  file = fopen(name, "rb");
  if (!file) {
    rb_sys_fail(name);
  }

  loader = rb_class_new_instance(0, NULL, cLoader);
  Data_Get_Struct(loader, ParsingContext, context);

  context->source_type = STREAM;
  context->source.stream.io = file;
  context->source.stream.close_on_exit = TRUE;

  return rb_ensure(load_documents, loader, destroy_context, loader);
}

static VALUE rb_libyaml_load_stream(VALUE self, VALUE obj) {
  ParsingContext* context;
  VALUE loader;

  loader = rb_class_new_instance(0, NULL, cLoader);
  Data_Get_Struct(loader, ParsingContext, context);

  switch (TYPE(obj)) {
    case T_STRING:
      context->source_type = STRING;
      context->source.string.str = (const unsigned char*)RSTRING_PTR(obj);
      context->source.string.length = RSTRING_LEN(obj);
      break;
    case T_FILE: {
      FILE* io = get_iostream(obj);
      if (io == NULL) {
        rb_raise(rb_eArgError, "Failed to get FILE object");
      }
      context->source_type = STREAM;
      context->source.stream.io = io;
      context->source.stream.close_on_exit = FALSE;
      break;
    }

    default:
      rb_raise(rb_eTypeError, "1st argument must be String or IO instances");
  }
  context->parse_all_documents = TRUE;

  return rb_ensure(load_documents, loader, destroy_context, loader);
}

static VALUE rb_libyaml_dump(VALUE self, VALUE robj, VALUE io) {
  NOT_IMPLEMENTED /* TODO */
  return robj;
}


static FILE* get_iostream(VALUE io) {
  struct RFile* rfile;
#ifdef RUBY_IO_H
  rb_io_t* open_file;
#else
  OpenFile* open_file;
#endif

  rfile = RFILE(io);
  if (rfile == NULL) {
    return NULL;
  }

  open_file = rfile->fptr;
  if (open_file == NULL) {
    return NULL;
  }

#ifdef RUBY_IO_H
  return rb_io_stdio_file(open_file);
#else
  return GetReadFile(open_file);
#endif
}
