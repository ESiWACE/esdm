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
 * @brief Entry point for ESDM API Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esdm-internal.h>
// TODO: Decide on initialization mechanism.
static int is_initialized = 0;

esdm_instance_t esdm = {
.procs_per_node = 1,
.total_procs = 1,
.config = NULL,
};

esdm_status esdm_set_procs_per_node(int procs) {
  assert(procs > 0);
  esdm.procs_per_node = procs;
  return ESDM_SUCCESS;
}

esdm_status esdm_set_total_procs(int procs) {
  assert(procs > 0);
  esdm.total_procs = procs;
  return ESDM_SUCCESS;
}

esdm_status esdm_load_config_str(const char *str) {
  assert(str != NULL);
  esdm.config = esdm_config_init_from_str(str);
  return ESDM_SUCCESS;
}

esdm_status esdm_dataset_get_dataspace(esdm_dataset_t *dset, esdm_dataspace_t **out_dataspace) {
  assert(dset != NULL);
  *out_dataspace = dset->dataspace;
  assert(*out_dataspace != NULL);
  return ESDM_SUCCESS;
}

esdm_status esdm_init() {
  ESDM_DEBUG("Init");

  if (!is_initialized) {
    ESDM_DEBUG("Initializing ESDM");

    //int status = atexit(esdm_atexit);

    // find configuration
    if (!esdm.config)
      esdm_config_init(&esdm);

    // optional modules (e.g. data and metadata backends)
    esdm_modules_init(&esdm);

    // core components
    esdm_layout_init(&esdm);
    esdm_performance_init(&esdm);
    esdm_scheduler_init(&esdm);

    ESDM_DEBUG_COM_FMT("ESDM", " esdm = {config = %p, modules = %p, scheduler = %p, layout = %p, performance = %p}\n",
    (void *)esdm.config,
    (void *)esdm.modules,
    (void *)esdm.scheduler,
    (void *)esdm.layout,
    (void *)esdm.performance);

    is_initialized = 1;

    ESDM_DEBUG("ESDM initialized and ready!");
  }

  return ESDM_SUCCESS;
}

esdm_status esdm_mkfs(int enforce_format, data_accessibility_t target) {
  if (!is_initialized) {
    return ESDM_ERROR;
  }
  int ret;
  int ret_final = ESDM_SUCCESS;
  if (esdm.modules->metadata_backend->config->data_accessibility == target) {
    ret = esdm.modules->metadata_backend->callbacks.mkfs(esdm.modules->metadata_backend, enforce_format);
    if (ret != ESDM_SUCCESS) {
      ret_final = ret;
    }
  }

  for (int i = 0; i < esdm.modules->data_backend_count; i++) {
    if (esdm.modules->data_backends[i]->config->data_accessibility == target) {
      ret = esdm.modules->data_backends[i]->callbacks.mkfs(esdm.modules->data_backends[i], enforce_format);
      if (ret != ESDM_SUCCESS) {
        ret_final = ret;
      }
    }
  }
  return ret_final;
}

esdm_status esdm_finalize() {
  ESDM_DEBUG(__func__);

  // ESDM data data structures that require proper cleanup..
  // in particular this effects data and cache state which is not yet persistent

  esdm_scheduler_finalize(&esdm);
  esdm_performance_finalize(&esdm);
  esdm_layout_finalize(&esdm);
  esdm_modules_finalize(&esdm);

  return ESDM_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Public API: POSIX Legacy Compaitbility /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

esdm_status esdm_stat(char *desc, char *result) {
  ESDM_DEBUG(__func__);

  esdm_init();

  return ESDM_SUCCESS;
}

esdm_status esdm_create(char *name, int mode, esdm_container_t **container, esdm_dataset_t **dataset) {
  ESDM_DEBUG(__func__);

  esdm_init();

  int64_t bounds[1] = {0};
  esdm_dataspace_t *dataspace;

  esdm_dataspace_create(1 /* 1D */, bounds, SMD_DTYPE_INT8, &dataspace);

  esdm_container_create(name, container);
  esdm_dataset_create(*container, "bytestream", dataspace, dataset);

  printf("Dataset 'bytestream' creation: %p\n", (void *)*dataset);

  esdm_dataset_commit(*dataset);
  esdm_container_commit(*container);

  return ESDM_SUCCESS;
}

esdm_status esdm_open(char *name, int mode) {
  ESDM_DEBUG(__func__);

  esdm_init();

  return ESDM_SUCCESS;
}

esdm_status esdm_write(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *out_subspace) {
  ESDM_DEBUG(__func__);
  assert(dataset);
  assert(buf);
  assert(out_subspace);

  return esdm_scheduler_process_blocking(&esdm, ESDM_OP_WRITE, dataset, buf, out_subspace);
}

esdm_status esdm_read(esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *subspace) {
  ESDM_DEBUG("");
  assert(dataset);
  assert(buf);
  assert(subspace);

  return esdm_scheduler_process_blocking(&esdm, ESDM_OP_READ, dataset, buf, subspace);
}

esdm_status esdm_close(void *desc) {
  ESDM_DEBUG(__func__);

  return ESDM_SUCCESS;
}

esdm_status esdm_sync() {
  ESDM_DEBUG(__func__);
  return ESDM_SUCCESS;
}
