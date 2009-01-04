/*
 * Utility macros
 */

#define RAISE_WITH_PARSER_INITIALIZE_FAILED \
  rb_sys_fail("Failed to initialize parser instance with yaml_parser_initialize()")


/*
 * constants
 */
#ifndef FALSE
# define FALSE 0
#endif
#ifndef TRUE
# define TRUE (~FALSE)
#endif


/*
 * function prototypes
 */
FILE* get_iostream(VALUE io);
VALUE ary_last(VALUE ary);

VALUE get_fixed_value_by_name(yaml_event_t* event);

VALUE do_parse(yaml_parser_t *p_parser);
VALUE do_parse_for_stream(yaml_parser_t *p_parser);

VALUE rb_libyaml_load(VALUE self, VALUE rstr);
VALUE rb_libyaml_load_file(VALUE self, VALUE file_str);
VALUE rb_libyaml_load_stream(VALUE self, VALUE rstr);
VALUE rb_libyaml_dump(VALUE self, VALUE robj, VALUE io);


