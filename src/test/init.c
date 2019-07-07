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

/*
 * This test exercises some routines used during initialization of ESDM.
 */

#include <test/util/test_util.h>


#include <esdm.h>
#include <stdio.h>
#include <stdlib.h>

char const* configString =
  "{"
    "\"esdm\": {"
      "\"backends\": [],"
      "\"metadata\": {"
        "\"type\": \"metadummy\","
        "\"name\": \"md\","
        "\"target\": \"./_metadummy\""
      "}"
    "}"
  "}";

int main(int argc, char const *argv[]) {
  eassert_crash(esdm_set_procs_per_node(-1));
  esdm_status status = esdm_set_procs_per_node(3);
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_set_total_procs(-1));
  status = esdm_set_total_procs(6);
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_load_config_str(NULL));
  eassert(esdm_load_config_str("") != ESDM_SUCCESS);
  status = esdm_load_config_str(configString);
  eassert(status == ESDM_SUCCESS);
  eassert_crash(esdm_load_config_str(configString));

  status = esdm_init();
  eassert(status == ESDM_SUCCESS);

  eassert_crash(esdm_set_procs_per_node(3));
  eassert_crash(esdm_set_total_procs(6));
  eassert_crash(esdm_load_config_str("[]"));

  status = esdm_finalize();
  eassert(status == ESDM_SUCCESS);
}
