#ifndef esdm_INC
#define esdm_INC


#include <sys/types.h>


char *esdm_suggest_tier(h5sqlite_fapl_t *fapl, int mpi_size, size_t total_bytes);


// ESDM Tiering Policy
typedef struct ESDM_policy_t {
  int min_total_bytes;
  int max_total_bytes;
  int min_nodes;
  int max_nodes;
  int min_tasks_per_node;
  int max_tasks_per_node;
  int tierid;
} ESDM_policy_t;


enum ESDM_tier {
  ESDM_TIER_SHM,
  ESDM_TIER_SSD,
  ESDM_TIER_HDD,
  ESDM_TIER_BURST,
  ESDM_TIER_LUSTRE,
  ESDM_TIER_LUSTRE_MULTIFILE,
};


#endif /* ----- #ifndef esdm_INC  ----- */
