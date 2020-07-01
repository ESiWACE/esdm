#ifndef ESDM_BACKENDS_S3_H
#define ESDM_BACKENDS_S3_H

#include <esdm-internal.h>
#include <backends-data/generic-perf-model/lat-thr.h>

#include <libs3.h>

/* Example esdm.conf section:
	"backends": [
		{
			"type": "S3",
			"id": "p1",
			"target": "./_posix1",
      "access-key" : "",
      "secret-key" : "",
      "host" : "localhost:9000"
		}
 */

// Internal functions used by this backend.
typedef struct {
  esdm_config_backend_t *config;
  esdm_perf_model_lat_thp_t perf_model;

  char const * access_key;
  char const * secret_key;
  char const * host;
  char const * bucket_prefix;
  char const * locationConstraint;
  char const * authRegion;

  int timeout;
  int s3_compatible;
  int use_ssl;
  S3Protocol s3_protocol;
} s3_backend_data_t;

int s3_finalize(esdm_backend_t *backend);
esdm_backend_t * s3_backend_init(esdm_config_backend_t *config);

#endif
