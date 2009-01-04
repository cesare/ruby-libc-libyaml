#include <ruby.h>
#include <rubyio.h>

#include <yaml.h>
#include <assert.h>

#include "libyaml.h"

#define NOT_IMPLEMENTED rb_raise(rb_eNotImpError, "not implemented!!");

static VALUE mYAML;
static VALUE mLibYAML;

void Init_libyaml()
{
  mYAML = rb_define_module("YAML");
  mLibYAML = rb_define_module_under(mYAML, "LibYAML");
  rb_define_singleton_method(mLibYAML, "load", rb_libyaml_load, 1);
  rb_define_singleton_method(mLibYAML, "load_file", rb_libyaml_load_file, 1);
  rb_define_singleton_method(mLibYAML, "load_stream", rb_libyaml_load_stream, 1);
  rb_define_singleton_method(mLibYAML, "dump", rb_libyaml_dump, 1);
}

VALUE rb_libyaml_load(VALUE self, VALUE rstr) {
  yaml_parser_t parser;
  VALUE results;
  int object_type = TYPE(rstr);
  
  if (object_type != T_STRING && object_type != T_FILE) {
    rb_raise(rb_eTypeError, "1st argument must be String or IO instances");
  }
  
  if (!yaml_parser_initialize(&parser)) {
    RAISE_WITH_PARSER_INITIALIZE_FAILED;
  }
  
  switch (TYPE(rstr)) {
    case T_STRING:
      yaml_parser_set_input_string(&parser, (unsigned char *)RSTRING_PTR(rstr), RSTRING_LEN(rstr));
      break;
    case T_FILE: {
      FILE* io = get_iostream(rstr);
      if (io == NULL) {
        yaml_parser_delete(&parser);
        rb_raise(rb_eArgError, "Failed to get FILE object");
      }
      yaml_parser_set_input_file(&parser, io);
      break;
    }
  }
  
  results = do_parse(&parser);
  
  yaml_parser_delete(&parser);
  return results;
}

VALUE rb_libyaml_load_file(VALUE self, VALUE file_str) {
  FILE *file;
  VALUE obj;
  yaml_parser_t parser;
  
  /* parameter file_str must be a string object */
  Check_Type(file_str, T_STRING);
  
  file = fopen(RSTRING_PTR(file_str), "rb");
  if ( !file ) {
    rb_sys_fail(RSTRING_PTR(file_str));
  }

  if (!yaml_parser_initialize(&parser)) {
    fclose(file);
    RAISE_WITH_PARSER_INITIALIZE_FAILED;
  }
  
  yaml_parser_set_input_file(&parser, file);

  obj = do_parse(&parser);
  assert(!fclose(file));
  
  yaml_parser_delete(&parser);
  return obj;
}

VALUE rb_libyaml_load_stream(VALUE self, VALUE rstr) {
  yaml_parser_t parser;
  yaml_event_t event;
  
  VALUE tmp_obj,obj;

  obj = rb_ary_new();

  if (!yaml_parser_initialize(&parser)) {
    RAISE_WITH_PARSER_INITIALIZE_FAILED;
  }
  
  yaml_parser_set_input_string(&parser, (unsigned char *)RSTRING_PTR(rstr), RSTRING_LEN(rstr));

  int done = 0;
  yaml_parser_parse(&parser, &event);
  if (event.type == YAML_STREAM_START_EVENT){
    while(!done){
      yaml_parser_parse(&parser, &event);
      if (event.type == YAML_DOCUMENT_START_EVENT) {
        tmp_obj = do_parse_for_stream(&parser);
        if (tmp_obj) {
          rb_ary_push(obj, tmp_obj);
        }
      } else if (event.type == YAML_STREAM_END_EVENT) {
        done = 1;
      } else {
        assert(0); /* TODO */
      }
    }
  }
  
  yaml_parser_delete(&parser);
  return obj;
}

VALUE rb_libyaml_dump(VALUE self, VALUE robj, VALUE io) {
  NOT_IMPLEMENTED /* TODO */
  return robj;
}

