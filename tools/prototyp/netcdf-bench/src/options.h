// Author: Julian Kunkel


#ifndef  options_INC
#define  options_INC

typedef enum{
  OPTION_FLAG,
  OPTION_OPTIONAL_ARGUMENT,
  OPTION_REQUIRED_ARGUMENT
} option_value_type;

typedef struct{
  char shortVar;
  char * longVar;
  char * help;

  option_value_type arg;
  char type;  // data type
  void * variable;
} option_help;

#define LAST_OPTION {0, 0, 0, (option_value_type) 0, 0, NULL}

void parseOptions(int argc, char ** argv, option_help * args);

#endif   /* ----- #ifndef options_INC  ----- */
