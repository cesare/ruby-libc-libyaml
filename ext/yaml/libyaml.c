#include <ruby.h>
#include <yaml.h>

#define NOT_IMPLEMENTED rb_raise(rb_eNotImpError, "not implemented!!");

static VALUE mYAML;
static VALUE mLibYAML;

VALUE LibYAML_load(VALUE self, VALUE rstr) {
  NOT_IMPLEMENTED /* TODO */
  return rstr;
}

VALUE LibYAML_load_file(VALUE self, VALUE file) {
  NOT_IMPLEMENTED /* TODO */
  return file;
}

VALUE LibYAML_load_stream(VALUE self, VALUE io) {
  NOT_IMPLEMENTED /* TODO */
  return io;
}

VALUE LibYAML_dump(VALUE self, VALUE robj, VALUE io) {
  NOT_IMPLEMENTED /* TODO */
  return robj;
}

void Init_libyaml()
{
  mYAML = rb_define_module("YAML");
  mLibYAML = rb_define_module_under(mYAML, "LibYAML");
  rb_define_singleton_method(mLibYAML, "load", LibYAML_load, 1);
  rb_define_singleton_method(mLibYAML, "load_file", LibYAML_load_file, 1);
  rb_define_singleton_method(mLibYAML, "load_stream", LibYAML_load_stream, 1);
  rb_define_singleton_method(mLibYAML, "dump", LibYAML_dump, 1);
}

