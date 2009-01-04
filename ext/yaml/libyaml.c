#include <ruby.h>
#include <rubyio.h>

#include <yaml.h>
#include <assert.h>

#include "libyaml.h"

#define NOT_IMPLEMENTED rb_raise(rb_eNotImpError, "not implemented!!");

static VALUE mYAML;
static VALUE mLibYAML;

VALUE ary_last(VALUE ary) {
  return rb_ary_entry(ary, RARRAY_LEN(ary) - 1);
}

VALUE do_parse(yaml_parser_t *p_parser) {
  yaml_event_t event;
  int done = 0;
  VALUE obj, stack, tmp_obj;
  char * num_reg_char = "^(\\+|-)?([0-9][0-9\\._]*)$";
  VALUE num_regex = rb_reg_new(num_reg_char, strlen(num_reg_char), NULL);
  char * hex_reg_char = "^0x[0-9a-fA-F]+";
  VALUE hex_regex = rb_reg_new(hex_reg_char, strlen(hex_reg_char), NULL);

  char * oct_reg_char = "^0[0-7]+$";
  VALUE oct_regex = rb_reg_new(oct_reg_char, strlen(oct_reg_char), NULL);

  obj     = (VALUE)NULL;
  stack = rb_ary_new3(1, rb_ary_new());

  while ( !done ) {
    yaml_parser_parse(p_parser, &event);
    done = ( event.type == YAML_STREAM_END_EVENT );

    switch( event.type ) {
      case YAML_SCALAR_EVENT:
        obj = rb_str_new2((char *)event.data.scalar.value);

        if (
            event.data.scalar.style != YAML_SINGLE_QUOTED_SCALAR_STYLE &&
            event.data.scalar.style != YAML_DOUBLE_QUOTED_SCALAR_STYLE
        ) {
          VALUE v = get_fixed_value_by_name(&event);
          if (v != Qundef) {
            obj = v;
          }
          else if ( rb_reg_match(oct_regex, obj) != Qnil ) {
            obj = rb_str_to_inum(obj, 8, Qfalse);
          } else if ( rb_reg_match(hex_regex, obj) != Qnil ) {
            obj = rb_str_to_inum(obj, 16, Qfalse);
          } else if ( rb_reg_match(num_regex, obj) != Qnil ) {
            obj = rb_Float(obj);
          }
        }

        tmp_obj = ary_last(stack);
        switch ( TYPE(tmp_obj) ) {
          case T_ARRAY:
            rb_ary_push(tmp_obj, obj);
            break;
          case T_HASH:
            rb_ary_push(stack, obj);
            break;
          default:
            tmp_obj = rb_ary_pop(stack);
            rb_hash_aset(ary_last(stack), tmp_obj, obj);
            obj = (VALUE)NULL;

            break;
        }
        tmp_obj = (VALUE)NULL;
        break;
      case YAML_SEQUENCE_START_EVENT:
        obj = rb_ary_new();
        rb_ary_push(stack, obj);
        break;
      case YAML_MAPPING_START_EVENT:
        obj = rb_hash_new();
        rb_ary_push(stack, obj);
        break;
      case YAML_SEQUENCE_END_EVENT:
      case YAML_MAPPING_END_EVENT:
        tmp_obj = rb_ary_pop(stack);
        switch ( TYPE( ary_last(stack) ) ) {
          case T_ARRAY:
            rb_ary_push(ary_last(stack), tmp_obj);
            break;
          case T_HASH:
            rb_ary_push(stack, tmp_obj);
            break;
          default:
            obj = rb_ary_pop(stack);
            assert(TYPE(ary_last(stack)) == T_HASH);
            rb_hash_aset(ary_last(stack), obj, tmp_obj);
            break;
        }
        tmp_obj = (VALUE)NULL;
        break;
      case YAML_NO_EVENT:
        /* TODO: it fall into loop. it may be better raise some Exception. */
        return Qnil;
        break;
      default:
        obj = Qnil;
        break;
    }

    yaml_event_delete(&event);
  }

  return ary_last(ary_last(stack));
}

VALUE do_parse_for_stream(yaml_parser_t *p_parser) {
  yaml_event_t event;
  int done = 0;
  VALUE obj, stack, tmp_obj;
  char * num_reg_char = "^(\\+|-)?([0-9][0-9\\._]*)$";
  VALUE num_regex = rb_reg_new(num_reg_char, strlen(num_reg_char), NULL);
  char * hex_reg_char = "^0x[0-9a-fA-F]+";
  VALUE hex_regex = rb_reg_new(hex_reg_char, strlen(hex_reg_char), NULL);

  char * oct_reg_char = "^0[0-7]+$";
  VALUE oct_regex = rb_reg_new(oct_reg_char, strlen(oct_reg_char), NULL);

  obj     = (VALUE)NULL;
  stack = rb_ary_new3(1, rb_ary_new());

  while ( !done ) {
    yaml_parser_parse(p_parser, &event);
    done = ( event.type == YAML_DOCUMENT_END_EVENT );
    //done = ( event.type == YAML_STREAM_END_EVENT );

    switch( event.type ) {
      case YAML_SCALAR_EVENT:
        obj = rb_str_new2((char *)event.data.scalar.value);

        if (
            event.data.scalar.style != YAML_SINGLE_QUOTED_SCALAR_STYLE &&
            event.data.scalar.style != YAML_DOUBLE_QUOTED_SCALAR_STYLE
        ) {
          VALUE v = get_fixed_value_by_name(&event);
          if (v != Qundef) {
            obj = v;
          }
          else if ( rb_reg_match(oct_regex, obj) != Qnil ) {
            obj = rb_str_to_inum(obj, 8, Qfalse);
          } else if ( rb_reg_match(hex_regex, obj) != Qnil ) {
            obj = rb_str_to_inum(obj, 16, Qfalse);
          } else if ( rb_reg_match(num_regex, obj) != Qnil ) {
            obj = rb_Float(obj);
          }
        }

        tmp_obj = ary_last(stack);
        switch ( TYPE(tmp_obj) ) {
          case T_ARRAY:
            rb_ary_push(tmp_obj, obj);
            break;
          case T_HASH:
            rb_ary_push(stack, obj);
            break;
          default:
            tmp_obj = rb_ary_pop(stack);
            rb_hash_aset(ary_last(stack), tmp_obj, obj);
            obj = (VALUE)NULL;

            break;
        }
        tmp_obj = (VALUE)NULL;
        break;
      case YAML_SEQUENCE_START_EVENT:
        obj = rb_ary_new();
        rb_ary_push(stack, obj);
        break;
      case YAML_MAPPING_START_EVENT:
        obj = rb_hash_new();
        rb_ary_push(stack, obj);
        break;
      case YAML_SEQUENCE_END_EVENT:
      case YAML_MAPPING_END_EVENT:
        tmp_obj = rb_ary_pop(stack);
        switch ( TYPE( ary_last(stack) ) ) {
          case T_ARRAY:
            rb_ary_push(ary_last(stack), tmp_obj);
            break;
          case T_HASH:
            rb_ary_push(stack, tmp_obj);
            break;
          default:
            obj = rb_ary_pop(stack);
            assert(TYPE(ary_last(stack)) == T_HASH);
            rb_hash_aset(ary_last(stack), obj, tmp_obj);
            break;
        }
        tmp_obj = (VALUE)NULL;
        break;
      case YAML_NO_EVENT:
        /* TODO: it fall into loop. it may be better raise some Exception. */
        return Qnil;
        break;
      default:
        obj = Qnil;
        break;
    }

    yaml_event_delete(&event);
  }

  return ary_last(ary_last(stack));
}

FILE* get_iostream(VALUE io) {
  struct RFile* rfile;
  OpenFile* open_file;
  FILE * file;
  
  rfile = RFILE(io);
  if (rfile == NULL) {
    return NULL;
  }
  
  open_file = rfile->fptr;
  if (open_file == NULL) {
    return NULL;
  }
  
  return GetReadFile(open_file);
}

VALUE rb_libyaml_load(VALUE self, VALUE rstr) {
  yaml_parser_t parser;
  VALUE results;
  int object_type = TYPE(rstr);
  
  if (object_type != T_STRING && object_type != T_FILE) {
    rb_raise(rb_eTypeError, "1st argument must be an String");
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

void Init_libyaml()
{
  mYAML = rb_define_module("YAML");
  mLibYAML = rb_define_module_under(mYAML, "LibYAML");
  rb_define_singleton_method(mLibYAML, "load", rb_libyaml_load, 1);
  rb_define_singleton_method(mLibYAML, "load_file", rb_libyaml_load_file, 1);
  rb_define_singleton_method(mLibYAML, "load_stream", rb_libyaml_load_stream, 1);
  rb_define_singleton_method(mLibYAML, "dump", rb_libyaml_dump, 1);
}

