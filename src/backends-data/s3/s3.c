#include <dirent.h>
#include <errno.h>
#include <esdm-debug.h>
#include <esdm.h>
#include <fcntl.h>
#include <jansson.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "s3.h"

/*
The mapping works as follows:
- upon mkfs a bucket with target == bucket-prefix is created
-- mkfs (format) will cleanup/remove all buckets with the given prefix including all keys in such buckets
- every dataset created creates one bucket with the name prefix serialized, e.g., user/test-hans is serialized to: <bucket-prefi>user-testhans
- upon write it is eagerly attempted to write to the bucket, if it doesn't work, the bucket is created
- theoretically, every fragment could be one key/value tuple with the (offset-size) N-D tuple serialized as "key", problem: if a offset-size tuple is overwritten, then data would be gone.
-- => Need to add random data to the key.
 */


#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("S3", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("S3", fmt, __VA_ARGS__)
#define DEBUG_S3(p, status) printf("S3 %d (object: %s) %s", __LINE__, p,  S3_get_status_name(status))


#define WARN_ENTER ESDM_WARN_COM_FMT("S3", "", "")
#define WARN(fmt, ...) ESDM_WARN_COM_FMT("S3", fmt, __VA_ARGS__)
#define WARNS(fmt) ESDM_WARN_COM_FMT("S3", "%s", fmt)

static void def_bucket_name(s3_backend_data_t * o, char * out_name, char const * path){
  out_name += sprintf(out_name, "%s-", o->bucket_prefix);
  // duplicate path except "/"
  while(*path != 0){
    char c = *path;
    if(((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') )){
      *out_name = *path;
      out_name++;
    }else if(c >= 'A' && c <= 'Z'){
      *out_name = *path + ('a' - 'A');
      out_name++;
    }
    path++;
  }
  *out_name = '\0';
}

static void init_bucket_context(char const * name, S3BucketContext * bucket_context, s3_backend_data_t *o){
  memset(bucket_context, 0, sizeof(*bucket_context));
  bucket_context->hostName = o->host;
  bucket_context->bucketName = name;
  bucket_context->protocol = o->s3_protocol;
  bucket_context->uriStyle = S3UriStylePath;
  bucket_context->accessKeyId = o->access_key;
  bucket_context->secretAccessKey = o->secret_key;
}

static S3Status responsePropertiesCallback(const S3ResponseProperties *properties, void *callbackData){
  //*(int*)callbackData = 1;
  return S3StatusOK;
}

static void responseCompleteCallback(S3Status status, const S3ErrorDetails *error, void *callbackData) {
  *(int*) callbackData = status;
  if (error != NULL){
    DEBUG("S3 error: %s", error->message);
  }
}

static S3ResponseHandler responseHandler = {  &responsePropertiesCallback, &responseCompleteCallback };

typedef struct {
  int status; // do not reorder!
  s3_backend_data_t * o;
} s3_req;

typedef struct {
  int status; // do not reorder!
  s3_backend_data_t * o;
  S3BucketContext * bucket_context;
  int truncated;
  char const *nextMarker;
} s3_delete_req;

S3Status list_delete_cb(int isTruncated, const char *nextMarker, int contentsCount, const S3ListBucketContent *contents, int commonPrefixesCount, const char **commonPrefixes, void *callbackData){
  int status;
  s3_delete_req * req = (s3_delete_req*) callbackData;
  for(int i=0; i < contentsCount; i++){
    S3_delete_object(req->bucket_context, contents[i].key, NULL, req->o->timeout, & responseHandler, & status);
  }
  req->truncated = isTruncated;
  if(isTruncated){
    req->nextMarker = nextMarker;
  }
  return S3StatusOK;
}

static S3ListBucketHandler list_delete_handler = {{&responsePropertiesCallback, &responseCompleteCallback }, list_delete_cb};


static S3Status S3RemoveBucketsCallback(const char *ownerId, const char *ownerDisplayName, const char *bucketName, int64_t creationDateSeconds, void *callbackData){
  s3_req * r = (s3_req*) callbackData;
  s3_backend_data_t * o = r->o;
  char const * a = bucketName;
  char const * b = o->bucket_prefix;
  // a must be a prefix of b
  while(*a == *b && *a != 0){
    a++;
    b++;
  }
  if(*b == 0 && *a != 0){
    DEBUG("delete \"%s\" (matches bucket-prefix: %s)\n", bucketName, r->o->bucket_prefix);
    int status;
    S3BucketContext bucket_context;
    init_bucket_context(bucketName, & bucket_context, o);
    s3_delete_req req = {0, o, & bucket_context, 0, NULL};
    do{
      S3_list_bucket(& bucket_context, NULL, req.nextMarker, NULL, INT_MAX, NULL, o->timeout, & list_delete_handler, & req);
    }while(req.truncated);

    S3_delete_bucket(o->s3_protocol, S3UriStylePath, o->access_key, o->secret_key, NULL, o->host, bucketName, o->authRegion, NULL, o->timeout, & responseHandler, & status);
    if( status != S3StatusOK){
      DEBUG_S3(bucketName, status);
    }
  } // otherwise doesn't match
  return S3StatusOK;
}

static S3ListServiceHandler remove_bucket_handler = { {  &responsePropertiesCallback, &responseCompleteCallback }, & S3RemoveBucketsCallback};

static int mkfs(esdm_backend_t *backend, int format_flags) {
  DEBUG("mkfs: backend S3\n", "");
  s3_backend_data_t *o = (s3_backend_data_t *)backend->data;

  int const ignore_err = format_flags & ESDM_FORMAT_IGNORE_ERRORS;

  int status = 0;
  if (format_flags & ESDM_FORMAT_DELETE) {
    S3_test_bucket(o->s3_protocol, S3UriStylePath, o->access_key, o->secret_key,
                        NULL, o->host, o->bucket_prefix, o->authRegion, 0, NULL,
                        NULL, o->timeout, & responseHandler, & status);
    if (status != S3StatusOK && ! ignore_err){
      DEBUG_S3(o->bucket_prefix, status);
      return ESDM_ERROR;
    }
    // delete all buckets with the same prefix
    s3_req req = { 0, o};
    S3_list_service(o->s3_protocol, o->access_key, o->secret_key, NULL, o->host, o->authRegion, NULL, o->timeout, & remove_bucket_handler, & req);
    if (req.status != S3StatusOK && ! ignore_err){
      DEBUG_S3(o->bucket_prefix, req.status);
      return ESDM_ERROR;
    }
    S3_delete_bucket(o->s3_protocol, S3UriStylePath, o->access_key, o->secret_key, NULL, o->host, o->bucket_prefix, o->authRegion, NULL, o->timeout, & responseHandler, & status);
    if (status != S3StatusOK && ! ignore_err){
      DEBUG_S3(o->bucket_prefix, status);
      return ESDM_ERROR;
    }
  }

  if(! (format_flags & ESDM_FORMAT_CREATE)){
    return ESDM_SUCCESS;
  }

  S3_create_bucket(o->s3_protocol, o->access_key, o->secret_key, NULL, o->host, o->bucket_prefix, o->authRegion, S3CannedAclPrivate, o->locationConstraint, NULL, o->timeout, & responseHandler, & status);
  if (status != S3StatusOK){
    DEBUG_S3(o->bucket_prefix, status);
    return ESDM_ERROR;
  }

  return ESDM_SUCCESS;
}

static int fsck(esdm_backend_t* backend) {
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Fragment Handlers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef struct{
  int status;
  int64_t size;
  char * buf;
} data_io_t;

static S3Status getObjectDataCallback(int bufferSize, const char *buffer,  void *callbackData){
  data_io_t * dh = (data_io_t*) callbackData;
  const int64_t size = dh->size > bufferSize ? bufferSize : dh->size;
  memcpy(dh->buf, buffer, size);
  dh->buf = dh->buf + size;
  dh->size -= size;

  return S3StatusOK;
}

static S3GetObjectHandler getObjectHandler = { {  &responsePropertiesCallback, &responseCompleteCallback }, & getObjectDataCallback };

static int fragment_retrieve(esdm_backend_t *backend, esdm_fragment_t *f) {
  DEBUG_ENTER;
  s3_backend_data_t *o = (s3_backend_data_t *)backend->data;
  //ensure that we have a contiguous read buffer
  void* readBuffer = f->dataspace->stride ? ea_checked_malloc(f->bytes) : f->buf;

  S3BucketContext bc;
  char bucketName[64];
  def_bucket_name(o, bucketName, f->dataset->id);
  init_bucket_context(bucketName, & bc, o);
  data_io_t dh = { .status = 0, .buf = readBuffer, .size = f->bytes };
  S3_get_object(& bc, f->id, NULL, 0, f->bytes, NULL, o->timeout, &getObjectHandler, & dh);
  if(dh.status != S3StatusOK){
    DEBUG_S3(f->id, dh.status);
    return ESDM_ERROR;
  }
  if(f->dataspace->stride) {
    //data is not necessarily supposed to be contiguous in memory -> copy from contiguous dataspace
    esdm_dataspace_t* contiguousSpace;
    esdm_dataspace_makeContiguous(f->dataspace, &contiguousSpace);
    esdm_dataspace_copy_data(contiguousSpace, readBuffer, f->dataspace, f->buf);
    esdm_dataspace_destroy(contiguousSpace);
    free(readBuffer);
  }

  return ESDM_SUCCESS;
}

static int putObjectDataCallback(int bufferSize, char *buffer, void *callbackData){
  // may be called with smaller packets
  data_io_t * dh = (data_io_t *) callbackData;
  const int64_t size = dh->size > bufferSize ? bufferSize : dh->size;
  if(size == 0) return 0;
  memcpy(buffer, dh->buf, size);
  dh->buf = dh->buf + size;
  dh->size -= size;

  return size;
}

static S3PutObjectHandler putObjectHandler = { {  &responsePropertiesCallback, &responseCompleteCallback }, & putObjectDataCallback };

static int fragment_update(esdm_backend_t *backend, esdm_fragment_t *f) {
  DEBUG_ENTER;
  s3_backend_data_t *o = (s3_backend_data_t *)backend->data;
  // set data, options and tgt for convenience
  int ret = ESDM_SUCCESS;
  //ensure that we have the data contiguously in memory
  void* writeBuffer = f->buf;
  if(f->dataspace->stride) {
    //data is not necessarily contiguous in memory -> copy to contiguous dataspace
    writeBuffer = ea_checked_malloc(f->bytes);
    esdm_dataspace_t* contiguousSpace;
    esdm_dataspace_makeContiguous(f->dataspace, &contiguousSpace);
    esdm_dataspace_copy_data(f->dataspace, f->buf, contiguousSpace, writeBuffer);
    esdm_dataspace_destroy(contiguousSpace);
  }
  // lazy assignment of ID
  if(f->id == NULL){
    // TODO make more unique, e.g., creating unique ID based on size + offset tuple
    f->id = ea_make_id(23);
  }
  S3BucketContext bc;
  char bucketName[64];
  def_bucket_name(o, bucketName, f->dataset->id);
  init_bucket_context(bucketName, & bc, o);
  data_io_t dh = { .status = 0, .buf = writeBuffer, .size = f->bytes };
  S3_put_object(& bc, f->id, f->bytes, NULL, NULL, o->timeout, &putObjectHandler, & dh);
  if(dh.status != S3StatusOK){
    int status;
    // may need to create bucket
    S3_create_bucket(o->s3_protocol, o->access_key, o->secret_key, NULL, o->host, bucketName, o->authRegion, S3CannedAclPrivate, o->locationConstraint, NULL, o->timeout, & responseHandler, & status);
    if (status != S3StatusOK){
      DEBUG_S3(o->bucket_prefix, status);
      // another peer may have created the bucket at the same time
      if ( status != S3StatusErrorBucketAlreadyExists && status != S3StatusErrorBucketAlreadyOwnedByYou){
        ret = ESDM_ERROR;
        goto cleanup;
      }
    }
    S3_put_object(& bc, f->id, f->bytes, NULL, NULL, o->timeout, &putObjectHandler, & dh);
    if(dh.status != S3StatusOK){
      DEBUG_S3(f->id, dh.status);
      ret = ESDM_ERROR;
    }
  }
  //cleanup
  cleanup:
  if(f->dataspace->stride) free(writeBuffer);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int s3_backend_performance_estimate(esdm_backend_t *backend, esdm_fragment_t *fragment, float *out_time) {
  DEBUG_ENTER;

  if (!backend || !fragment || !out_time)
    return 1;

  s3_backend_data_t *data = (s3_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_long_lat_perf_estimate(&data->perf_model, fragment, out_time);
}

static float s3_backend_estimate_throughput(esdm_backend_t* backend) {
  DEBUG_ENTER;

  s3_backend_data_t *data = (s3_backend_data_t *)backend->data;
  return esdm_backend_t_perf_model_get_throughput(&data->perf_model);
}

static int fragment_delete(esdm_backend_t * backend, esdm_fragment_t *f){
  return ESDM_SUCCESS;
}

int s3_finalize(esdm_backend_t *backend) {
  DEBUG_ENTER;

  s3_backend_data_t* data = backend->data;
  free(data->config);  //TODO: Do we need to destruct this?
  free(data);
  free(backend);

  S3_deinitialize();

  return 0;
}



///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend_t backend_template = {
  ///////////////////////////////////////////////////////////////////////////////
  // NOTE: This serves as a template for the dummy plugin and is memcopied!    //
  ///////////////////////////////////////////////////////////////////////////////
  .name = "S3",
  .type = ESDM_MODULE_DATA,
  .version = "0.0.1",
  .data = NULL,
  .callbacks = {
    .finalize = s3_finalize,
    .performance_estimate = s3_backend_performance_estimate,
    .estimate_throughput = s3_backend_estimate_throughput,
    .fragment_create = NULL,
    .fragment_retrieve = fragment_retrieve,
    .fragment_update = fragment_update,
    .fragment_delete = fragment_delete,
    .fragment_metadata_create = NULL,
    .fragment_metadata_load = NULL,
    .fragment_metadata_free = NULL,
    .mkfs = mkfs,
    .fsck = fsck,
  },
};

#define JSON_REQUIRE(what) \
  j = jansson_object_get(config->backend, what); \
  if(! j){ \
    ESDM_ERROR("Configuration: " what " not set");\
  }\

esdm_backend_t *s3_backend_init(esdm_config_backend_t *config) {
  DEBUG_ENTER;

  if (!config || !config->type || strcasecmp(config->type, "S3") || !config->target) {
    DEBUG("Wrong configuration%s\n", "");
    return NULL;
  }

  esdm_backend_t *backend = ea_checked_malloc(sizeof(esdm_backend_t));
  memcpy(backend, &backend_template, sizeof(esdm_backend_t));

  // allocate memory for backend instance
  s3_backend_data_t *data = ea_checked_malloc(sizeof(*data));
  memset(data, 0, sizeof(*data));
  backend->data = data;

  if (data && config->performance_model)
    esdm_backend_t_parse_perf_model_lat_thp(config->performance_model, &data->perf_model);
  else
    esdm_backend_t_reset_perf_model_lat_thp(&data->perf_model);

  json_t * j;
  JSON_REQUIRE("access-key")
  data->access_key = json_string_value(j);
  JSON_REQUIRE("secret-key")
  data->secret_key = json_string_value(j);
  j = jansson_object_get(config->backend, "host");
  if(j) data->host = json_string_value(j);
  j = jansson_object_get(config->backend, "target");
  if(j) data->bucket_prefix = json_string_value(j);
  else data->bucket_prefix = "";
  if(strlen(data->bucket_prefix) < 5){
    DEBUG("Target must be longer than 5 characters (this is the bucket name prefix). Given: \"%s\"\n", j);
    return NULL;
  }
  j = jansson_object_get(config->backend, "locationConstraint");
  if(j) data->locationConstraint = json_string_value(j);
  j = jansson_object_get(config->backend, "authRegion");
  if(j)  data->authRegion = json_string_value(j);
  j = jansson_object_get(config->backend, "timeout");
  if(j) data->timeout = json_integer_value(j);
  j = jansson_object_get(config->backend, "s3-compatible");
  if(j) data->s3_compatible = json_integer_value(j);
  j = jansson_object_get(config->backend, "use-ssl");
  if(j) data->use_ssl = json_integer_value(j);
  if(data->use_ssl){
    data->s3_protocol = S3ProtocolHTTPS;
  }else{
    data->s3_protocol  = S3ProtocolHTTP;
  }

  // configure backend instance
  data->config = config;

  int ret = S3_initialize(NULL, S3_INIT_ALL, data->host);
  if(ret != S3StatusOK){
    WARNS("Could not initialize S3 library");
  }

  return backend;
}
