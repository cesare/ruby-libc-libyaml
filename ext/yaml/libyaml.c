#include <ruby.h>
#include <yaml.h>

#define NOT_IMPLEMENTED rb_raise(rb_eNotImpError, "not implemented!!");

static VALUE mYAML;
static VALUE mLibYAML;

VALUE rb_libyaml_load(VALUE self, VALUE rstr) {
  NOT_IMPLEMENTED /* TODO */
  return rstr;
}

VALUE rb_libyaml_load_file(VALUE self, VALUE file) {
  NOT_IMPLEMENTED /* TODO */
  return file;
}

VALUE rb_libyaml_load_stream(VALUE self, VALUE io) {
  NOT_IMPLEMENTED /* TODO */
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

