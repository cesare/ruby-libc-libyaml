#include <ruby.h>

#ifdef RUBY_RUBY_H
# include <ruby/encoding.h>
#endif

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

static VALUE wrap_rb_utf8_str_new(const char* str, long length) {
#ifdef RUBY_ENCODING_H
  /* TODO set encoding properly */
  return rb_enc_str_new(str, length, rb_enc_find("UTF-8"));
#else
  return rb_str_new(str, length);
#endif
}

static VALUE is_valid_string_as(const char* str, const char** pattern, int length) {
  int i;
  
  for (i = 0; i < length; i++) {
    if (strcmp(str, pattern[i]) == 0) {
      return Qtrue;
    }
  }
  
  return Qfalse;
}

static VALUE get_fixed_value_by_name(const char* str) {
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

static VALUE get_fixnum_by_regexp(VALUE rstring) {
  if ( rb_reg_match(rb_const_get(mLibYAML, rb_intern("OCT_REGEX")), rstring) != Qnil ) {
    return rb_str_to_inum(rstring, 8, Qfalse);
  }
  
  if ( rb_reg_match(rb_const_get(mLibYAML, rb_intern("HEX_REGEX")), rstring) != Qnil ) {
    return rb_str_to_inum(rstring, 16, Qfalse);
  }
  
  if ( rb_reg_match(rb_const_get(mLibYAML, rb_intern("NUM_REGEX")), rstring) != Qnil ) {
    const char* str = RSTRING_PTR(rstring);
    return (strchr(str, '.') == NULL && strchr(str, 'e') == NULL && strchr(str, 'E') == NULL ) ? rb_str_to_inum(rstring, 10, Qfalse) : rb_Float(rstring);
  }
  
  return Qundef;
}

static VALUE get_symbol(VALUE rstring) {
  const char* pattern = "^:.+$";
  const VALUE regexp = rb_reg_new(pattern, strlen(pattern), 0);
  const char* str;
  
  if (rb_reg_match(regexp, rstring) == Qnil) {
    return Qundef;
  }
  
  str = RSTRING_PTR(rstring);
  str++; /* length of rstring is determined more than 2 bytes, as it matches regexp above */
  
  return ID2SYM(rb_intern(str));
}

static VALUE construct_scalar_node(yaml_document_t* document, yaml_node_t* node) {
  VALUE value;
  const char* str;
  size_t length;
  VALUE rstring = Qundef;
  yaml_scalar_style_t style = node->data.scalar.style;
  
  str = (const char*)(node->data.scalar.value);
  length = node->data.scalar.length;
  
  if (style == YAML_SINGLE_QUOTED_SCALAR_STYLE || style == YAML_DOUBLE_QUOTED_SCALAR_STYLE) {
    goto DEFAULT;
  }
  
  value = get_fixed_value_by_name(str);
  if (value != Qundef) {
    return value;
  }

  rstring = wrap_rb_utf8_str_new(str, length);

  value = get_symbol(rstring);
  if (value != Qundef) {
    return value;
  }
  
  value = get_fixnum_by_regexp(rstring);
  if (value != Qundef) {
    return value;
  }
  
  DEFAULT:
  if (rstring == Qundef) {
    rstring = wrap_rb_utf8_str_new(str, length);
  }
  return rstring;
}

static VALUE construct_sequence_node(yaml_document_t* document, yaml_node_t* node) {
  yaml_node_item_t* start = node->data.sequence.items.start;
  yaml_node_item_t* top   = node->data.sequence.items.top;
  yaml_node_item_t* item;
  VALUE results;
  int i;
  
  results = rb_ary_new2(top - start);
  i = 0;
  for (item = start; item < top; item++) {
    VALUE elements = construct_node(document, yaml_document_get_node(document, *item));
    rb_ary_store(results, i, elements);
    i++;
  }
  
  return results;
}

static VALUE construct_mapping_node(yaml_document_t* document, yaml_node_t* node) {
  yaml_node_pair_t* start = node->data.mapping.pairs.start;
  yaml_node_pair_t* top   = node->data.mapping.pairs.top;
  yaml_node_pair_t* pair;
  VALUE results;
  
  results = rb_hash_new();
  for (pair = start; pair < top; pair++) {
    VALUE key   = construct_node(document, yaml_document_get_node(document, pair->key));
    VALUE value = construct_node(document, yaml_document_get_node(document, pair->value));
    rb_hash_aset(results, key, value);
  }
  
  return results;
}

VALUE construct_node(yaml_document_t* document, yaml_node_t* node) {
  switch (node->type) {
    case YAML_NO_NODE:
      return Qnil;  /* TODO correct? */
    case YAML_SCALAR_NODE:
      return construct_scalar_node(document, node);
    case YAML_SEQUENCE_NODE:
      return construct_sequence_node(document, node);
    case YAML_MAPPING_NODE:
      return construct_mapping_node(document, node);
  }
  return Qnil;
}
