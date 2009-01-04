/*
 * Utility macros
 */

#define RAISE_WITH_PARSER_INITIALIZE_FAILED \
  rb_sys_fail("Failed to initialize parser instance with yaml_parser_initialize()")



/*
 * function prototypes
 */
VALUE get_fixed_value_by_name(yaml_event_t* event);
