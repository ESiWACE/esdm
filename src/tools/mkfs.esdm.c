#include <stdio.h>
#include <stdlib.h>

#include <tools/option.h>

#include <esdm.h>

typedef struct{
  char * config_file;
  int verbosity;
  int enforce_format;
  int remove_data;
} tool_options_t;

static tool_options_t o = {
  .config_file = "esdm.conf",
  .verbosity = 0,
  .enforce_format = 0,
  .remove_data = 0,
};

void parse_args(int argc, char** argv){
  option_help options[] = {
    {'c', "config", "The configuration file", OPTION_OPTIONAL_ARGUMENT, 's', & o.config_file},
    {'v', NULL, "Increase verbosity", OPTION_FLAG, 'd', & o.verbosity},
    {0, "remove-only", "WARNING: remove existing data", OPTION_FLAG, 'd', & o.remove_data},
    {0, "force-format", "WARNING: enforce the formatting", OPTION_FLAG, 'd', & o.enforce_format},
    {0, "verbosity", "Set verbosity", OPTION_OPTIONAL_ARGUMENT, 'd', & o.verbosity},
    LAST_OPTION
  };

  int ret;
  int print_help = 0;
  ret = option_parse(argc, argv, options, & print_help);
  if(print_help){
    option_print_help(options, 0);
    exit(0);
  }
}

int main(int argc, char** argv){
  parse_args(argc, argv);

	char * config = NULL;
  read_file(o.config_file, & config);
	esdm_load_config_str(config);

  esdm_status_t ret;

  ret = esdm_init();
  ret = esdm_mkfs(o.remove_data ? 2 : o.enforce_format);
  if (ret != ESDM_SUCCESS){
    printf("Error during mkfs!\n");
  }
  ret = esdm_finalize();
  return 0;
}
