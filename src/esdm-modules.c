#ifndef ESDM_MODULES_H
#define ESDM_MODULES_H

#ifdef ESDM_HAS_POSIX
#include <data-backends/posix/posix.h>
#endif

static int module_count;
static esdm_module_t modules[] = {
#ifdef ESDM_HAS_POSIX
  & esdm_posix,
#endif
  NULL
};

typedef enum {
  ESDM_TYPE_BACKEND,
  ESDM_TYPE_METADATA,
  ESDM_TYPE_LAST
} esdm_module_type_t;

typedef {
  int count;
  esdm_module_t * module;
} esdm_module_type_array_t;

static esdm_module_type_array_t modules_per_type[ESDM_TYPE_LAST];

ESDM_status_t esdm_module_init(){
  module_count = 0;
  esdm_module_t * cur = modules;
  for( ; cur != NULL ; cur++){
    cur->init();
    modules_per_type[cur->type()]++;
  }
  // place the module into the right list
}

ESDM_status_t esdm_module_finalize(){
  // reverse finalization of modules
  for(int i=module_count - 1 ; i >= 0; i--){
    modules[i]->finalize();
  }
}

esdm_module_type_array_t * esdm_module_get_per_type(esdm_module_type_t type){
}


#endif
