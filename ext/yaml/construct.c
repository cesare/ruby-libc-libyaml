#include <ruby.h>
#include <rubyio.h>
#include <assert.h>
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
  
  return Qundef;
}

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
    done = ( event.type == YAML_DOCUMENT_END_EVENT );

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


