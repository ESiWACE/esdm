#ifndef ESDM_BACKENDS_S3_H
#define ESDM_BACKENDS_S3_H

#include <esdm-internal.h>

#include <backends-data/generic-perf-model/lat-thr.h>

// Internal functions used by this backend.
typedef struct {
  esdm_config_backend_t *config;
  esdm_perf_model_lat_thp_t perf_model;
} s3_backend_data_t;

int s3_finalize(esdm_backend_t *backend);
esdm_backend_t * s3_backend_init(esdm_config_backend_t *config);

#endif
