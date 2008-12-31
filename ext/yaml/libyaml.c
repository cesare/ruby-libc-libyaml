#include <ruby.h>
#include <yaml.h>
#include <assert.h>

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
  char * num_reg_char = "^(\\+|-)?([0-9][0-9\\._]+)$";
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

        if ( RSTRING_LEN(obj) == 1 && RSTRING_PTR(obj)[0] == '~' ) {
          obj = Qnil;
        } else if ( rb_reg_match(oct_regex, obj) != Qnil ) {
          obj = rb_str_to_inum(obj, 8, Qfalse);
        } else if ( rb_reg_match(hex_regex, obj) != Qnil ) {
          obj = rb_str_to_inum(obj, 16, Qfalse);
        } else if ( rb_reg_match(num_regex, obj) != Qnil ) {
          obj = rb_Float(obj);
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

  yaml_parser_delete(p_parser);
  return ary_last(ary_last(stack));
}

VALUE rb_libyaml_load(VALUE self, VALUE rstr) {
  yaml_parser_t parser;

  assert(yaml_parser_initialize(&parser));

  yaml_parser_set_input_string(&parser, (unsigned char *)RSTRING_PTR(rstr), RSTRING_LEN(rstr));
  return do_parse(&parser);
}

VALUE rb_libyaml_load_file(VALUE self, VALUE file_str) {
  FILE *file;
  VALUE obj;
  yaml_parser_t parser;

  assert(yaml_parser_initialize(&parser));
  file = fopen(RSTRING_PTR(file_str), "rb");
  assert(file);
  yaml_parser_set_input_file(&parser, file);

  obj = do_parse(&parser);
  assert(!fclose(file));

  return obj;
}

VALUE rb_libyaml_load_stream(VALUE self, VALUE io) {
  NOT_IMPLEMENTED /* TODO: */
  return io;
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

