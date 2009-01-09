/*
 * constants
 */
#ifndef FALSE
# define FALSE 0
#endif
#ifndef TRUE
# define TRUE (~FALSE)
#endif

typedef int BOOL;


/*
 * data types
 */

typedef enum {
  STRING,
  STREAM
} InputSourceType;

typedef struct  {
  yaml_parser_t* parser;
  
  InputSourceType source_type;
  
  union {
    struct {
      const unsigned char* str;
      size_t length;
    } string; /* TODO choose another name */

    struct {
      FILE* io;
      BOOL close_on_exit;
    } stream;
    
  } source;
  
  BOOL parse_all_documents;
  
} ParsingContext;


/*
 * function prototypes
 */

VALUE construct_node(yaml_document_t* document, yaml_node_t* node);

