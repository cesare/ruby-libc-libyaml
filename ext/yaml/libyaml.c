#include <ruby.h>
#include <yaml.h>
#include <assert.h>

#define NOT_IMPLEMENTED rb_raise(rb_eNotImpError, "not implemented!!");

static VALUE mYAML;
static VALUE mLibYAML;

VALUE rb_libyaml_load(VALUE self, VALUE rstr) {
  NOT_IMPLEMENTED /* TODO */
  return rstr;
}

VALUE rb_libyaml_load_file(VALUE self, VALUE file_str) {
  FILE *file;
  yaml_parser_t parser;
  yaml_event_t event;
  int done = 0;
  VALUE ary, obj, stack, tmp_ary;
  
  assert(yaml_parser_initialize(&parser));
  file = fopen(RSTRING_PTR(file_str), "rb");
  assert(file);
  yaml_parser_set_input_file(&parser, file);

  ary = rb_ary_new();
  obj     = (VALUE)NULL;
  stack = rb_ary_new();

  while ( !done ) {
    yaml_parser_parse(&parser, &event);
    done = ( event.type == YAML_STREAM_END_EVENT );

    switch( event.type ) {
      case YAML_SCALAR_EVENT:
        obj = rb_str_new2((char *)event.data.scalar.value);

        if ( RARRAY_LEN(stack) ) {
          rb_ary_push(rb_ary_entry(stack, RARRAY_LEN(stack) - 1), obj);
        } else {
          rb_ary_push(ary, obj);
        }
        break;
      case YAML_SEQUENCE_START_EVENT:
        obj = rb_ary_new();
        rb_ary_push(stack, obj);
        break;
      case YAML_SEQUENCE_END_EVENT:
        tmp_ary = rb_ary_pop(stack);
        if ( RARRAY_LEN(stack) ) {
          rb_ary_push(rb_ary_entry(stack, RARRAY_LEN(stack) - 1), tmp_ary);
        } else {
          rb_ary_push(ary, tmp_ary);
        }
        break;
      default:
        obj = Qnil;
        break;
    }

    yaml_event_delete(&event);
  }

  yaml_parser_delete(&parser);
  assert(!fclose(file));
  return ary;
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

