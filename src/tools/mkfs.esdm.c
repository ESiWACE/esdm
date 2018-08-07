#include <stdio.h>
#include <stdlib.h>
//#include <mpi.h>

#include <esdm.h>
#include <tools/option.h>

typedef struct{
  char * config_file;
  int verbosity;
  int enforce_format;
  int remove_data;
  int format_local;
  int format_global;
} tool_options_t;

static int rank;

static tool_options_t o = {
  .config_file = "esdm.conf",
  .verbosity = 0,
  .enforce_format = 0,
  .remove_data = 0,
  .format_local = 0,
  .format_global = 0
};

void parse_args(int argc, char** argv){
  option_help options[] = {
    {'g', "global", "Formatting global storage system", OPTION_FLAG, 'd', & o.format_global},
    {'l', "local", "Formatting local storage systems", OPTION_FLAG, 'd', & o.format_local},
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
    if(rank == 0) option_print_help(options, 0);
    exit(0);
  }
}

int main(int argc, char** argv){
  parse_args(argc, argv);
  if(o.format_global + o.format_local == 0){
    printf("Not formatting anything, use -g and/or -l\n");
    exit(1);
  }

	char * config = NULL;
  read_file(o.config_file, & config);
	esdm_load_config_str(config);

  esdm_status_t ret;
  ret = esdm_init();
  if(o.format_global){
    ret = esdm_mkfs(o.remove_data ? 2 : o.enforce_format, ESDM_ACCESSIBILITY_GLOBAL);
    if (ret != ESDM_SUCCESS){
      printf("Error during mkfs!\n");
    }
  }
  if(o.format_local){
    ret = esdm_mkfs(o.remove_data ? 2 : o.enforce_format, ESDM_ACCESSIBILITY_NODELOCAL);
    if (ret != ESDM_SUCCESS){
      printf("Error during mkfs!\n");
    }
  }
  ret = esdm_finalize();
  return 0;
}
