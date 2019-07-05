/* This file is part of ESDM.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief Format storage target for ESDM from configuration file.
 */

#include <esdm-internal.h>
#include <stdio.h>
#include <stdlib.h>
#include <tools/option.h>
//#include <mpi.h>

typedef struct {
  char *config_file;
  int verbosity;
  int ignore_errors;
  int create_data;
  int delete_data;
  int format_local;
  int format_global;
} tool_options_t;

static int rank;

static tool_options_t o = {
.config_file = "esdm.conf",
.verbosity = 0,
.ignore_errors = 0,
.create_data = 0,
.delete_data = 0,
.format_local = 0,
.format_global = 0};

void parse_args(int argc, char **argv) {
  option_help options[] = {
  {'g', "global", "Formatting global storage system", OPTION_FLAG, 'd', &o.format_global},
  {'l', "local", "Formatting local storage systems", OPTION_FLAG, 'd', &o.format_local},
  {'c', "config", "The configuration file", OPTION_OPTIONAL_ARGUMENT, 's', &o.config_file},
  {'v', NULL, "Increase verbosity", OPTION_FLAG, 'd', &o.verbosity},
  {0, "create", "WARNING: create data repository", OPTION_FLAG, 'd', &o.create_data},
  {0, "remove", "WARNING: remove existing data", OPTION_FLAG, 'd', &o.delete_data},
  {0, "ignore-errors", "WARNING: ignore errors that appear during create/delete", OPTION_FLAG, 'd', &o.ignore_errors},
  {0, "verbosity", "Set verbosity", OPTION_OPTIONAL_ARGUMENT, 'd', &o.verbosity},
  LAST_OPTION};

  int ret;
  int print_help = 0;
  ret = option_parse(argc, argv, options, &print_help);
  if (print_help) {
    if (rank == 0) option_print_help(options, 0);
    exit(0);
  }
}

int main(int argc, char **argv) {
  parse_args(argc, argv);
  if (o.format_global + o.format_local == 0) {
    printf("Not formatting anything, use -g and/or -l\n");
    exit(1);
  }

  char *config = NULL;
  read_file(o.config_file, &config);
  esdm_load_config_str(config);

  esdm_status ret;
  ret = esdm_init();
  int flags = (o.create_data ? ESDM_FORMAT_CREATE : 0) |
              (o.delete_data ? ESDM_FORMAT_DELETE : 0) |
              (o.ignore_errors ? ESDM_FORMAT_IGNORE_ERRORS : 0);
  if(flags == 0){
    printf("MKFS: Nothing to do. Use --create and/or --remove\n");
    exit(1);
  }
  if (o.format_global) {
    ret = esdm_mkfs(flags, ESDM_ACCESSIBILITY_GLOBAL);
    if (ret != ESDM_SUCCESS) {
      printf("Error during mkfs -g!\n");
      exit(1);
    }
  }
  if (o.format_local) {
    ret = esdm_mkfs(flags, ESDM_ACCESSIBILITY_NODELOCAL);
    if (ret != ESDM_SUCCESS) {
      printf("Error during mkfs -l!\n");
      exit(1);
    }
  }
  ret = esdm_finalize();
  printf("[mkfs] OK\n");
  return 0;
}
