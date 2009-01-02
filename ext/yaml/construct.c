#include <ruby.h>
#include <yaml.h>

#include "libyaml.h"

static const char* VALID_NULL_STRINGS[] = {
  "~",
  "null",
  "Null",
  "NULL"
};

static const char* VALID_TRUE_STRINGS[] = {
  "true",
  "True",
  "TRUE"
};

static const char* VALID_FALSE_STRINGS[] = {
  "false",
  "False",
  "FALSE"
};

static const char* VALID_NAN_STRINGS[] = {
  ".nan",
  ".NaN",
  ".NAN"
};

static const char* VALID_INFINITY_STRINGS[] = {
  ".inf",
  ".Inf",
  ".INF"
};

static const float POSITIVE_INFINITY =  (1.0 / 0.0);
static const float NEGATIVE_INFINITY = -(1.0 / 0.0);
static const float NOT_A_NUMBER      =  (0.0 / 0.0);


#define is_valid_as(s,a) is_valid_string_as((s), (a), (sizeof((a)) / sizeof(const char*)))

static VALUE is_valid_string_as(const char* str, const char** pattern, int length) {
  int i;
  
  for (i = 0; i < length; i++) {
    if (strcmp(str, pattern[i]) == 0) {
      return Qtrue;
    }
  }
  
  return Qfalse;
}

VALUE get_fixed_value_by_name(yaml_event_t* event) {
  const char* str  = (const char*)(event->data.scalar.value);
  const char* ptr  = NULL;
  const char* head = NULL;
  
  VALUE rbstr;
  char * num_reg_char = "^(\\+|-)?([0-9][0-9\\._]+)$";
  VALUE num_regex = rb_reg_new(num_reg_char, strlen(num_reg_char), NULL);
  char * hex_reg_char = "^0x[0-9a-fA-F]+";
  VALUE hex_regex = rb_reg_new(hex_reg_char, strlen(hex_reg_char), NULL);
  char * oct_reg_char = "^0[0-7]+$";
  VALUE oct_regex = rb_reg_new(oct_reg_char, strlen(oct_reg_char), NULL);

  if (str == NULL) {
    return Qundef;
  }
  
  if (is_valid_as(str, VALID_NULL_STRINGS)) {
    return Qnil;
  }
  if (is_valid_as(str, VALID_TRUE_STRINGS)) {
    return Qtrue;
  }
  
  if (is_valid_as(str, VALID_FALSE_STRINGS)) {
    return Qfalse;
  }
  
  if (is_valid_as(str, VALID_NAN_STRINGS)) {
    return rb_float_new(NOT_A_NUMBER);
  }
  
  head = str;
  ptr = (*head == '+' || *head == '-') ? head + 1 : head;
  if (is_valid_as(ptr, VALID_INFINITY_STRINGS)) {
    return rb_float_new(*head == '-' ? NEGATIVE_INFINITY : POSITIVE_INFINITY);
  }
  
  rbstr = rb_str_new2(str);
  if ( rb_reg_match(oct_regex, rbstr) != Qnil ) {
    return rb_str_to_inum(rbstr, 8, Qfalse);
  } else if ( rb_reg_match(hex_regex, rbstr) != Qnil ) {
    return rb_str_to_inum(rbstr, 16, Qfalse);
  } else if ( rb_reg_match(num_regex, rbstr) != Qnil ) {
    return rb_Float(rbstr);
  }

  return Qundef;
}


