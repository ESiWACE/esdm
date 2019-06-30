
#include <stdio.h>  //
#include <stdlib.h> // getenv(),
#include <string.h> // strcpy(), strcat(),
#include <unistd.h> //


#include "esdm.h"
#include "h5_sqlite_plugin.h"

ESDM_policy_t policies[] = {

// 2MiB /////////
// Nodes=1
// Nodes=2
{.min_total_bytes = 0, .max_total_bytes = 1024 * 1024 * 5, .min_nodes = 0, .max_nodes = 3, .min_tasks_per_node = 0, .max_tasks_per_node = 5, .tierid = ESDM_TIER_SHM},
{.min_total_bytes = 0, .max_total_bytes = 1024 * 1024 * 5, .min_nodes = 0, .max_nodes = 3, .min_tasks_per_node = 5, .max_tasks_per_node = 70, .tierid = ESDM_TIER_SSD},

// Nodes=4 oscillate wildy
{.min_total_bytes = 0, .max_total_bytes = 1024 * 1024 * 5, .min_nodes = 3, .max_nodes = 5, .min_tasks_per_node = 0, .max_tasks_per_node = 2, .tierid = ESDM_TIER_LUSTRE},
{.min_total_bytes = 0, .max_total_bytes = 1024 * 1024 * 5, .min_nodes = 3, .max_nodes = 5, .min_tasks_per_node = 2, .max_tasks_per_node = 4, .tierid = ESDM_TIER_SHM},
{.min_total_bytes = 0, .max_total_bytes = 1024 * 1024 * 5, .min_nodes = 3, .max_nodes = 5, .min_tasks_per_node = 4, .max_tasks_per_node = 8, .tierid = ESDM_TIER_SSD},
{.min_total_bytes = 0, .max_total_bytes = 1024 * 1024 * 5, .min_nodes = 3, .max_nodes = 5, .min_tasks_per_node = 8, .max_tasks_per_node = 16, .tierid = ESDM_TIER_SHM},
{.min_total_bytes = 0, .max_total_bytes = 1024 * 1024 * 5, .min_nodes = 3, .max_nodes = 5, .min_tasks_per_node = 16, .max_tasks_per_node = 32, .tierid = ESDM_TIER_LUSTRE},
{.min_total_bytes = 0, .max_total_bytes = 1024 * 1024 * 5, .min_nodes = 3, .max_nodes = 5, .min_tasks_per_node = 32, .max_tasks_per_node = 64, .tierid = ESDM_TIER_SSD},

// Nodes=8
// Nodes=16
{.min_total_bytes = 0, .max_total_bytes = 1024 * 1024 * 5, .min_nodes = 5, .max_nodes = 20, .min_tasks_per_node = 0, .max_tasks_per_node = 5, .tierid = ESDM_TIER_SHM},
{.min_total_bytes = 0, .max_total_bytes = 1024 * 1024 * 5, .min_nodes = 5, .max_nodes = 20, .min_tasks_per_node = 5, .max_tasks_per_node = 70, .tierid = ESDM_TIER_SSD},


// 16 MiB /////////
// Nodes=1
// Nodes=2
// Nodes=4
{.min_total_bytes = 1024 * 1024 * 5, .max_total_bytes = 1024 * 1024 * 20, .min_nodes = 0, .max_nodes = 5, .min_tasks_per_node = 0, .max_tasks_per_node = 5, .tierid = ESDM_TIER_SHM},
{.min_total_bytes = 1024 * 1024 * 5, .max_total_bytes = 1024 * 1024 * 20, .min_nodes = 0, .max_nodes = 5, .min_tasks_per_node = 5, .max_tasks_per_node = 64, .tierid = ESDM_TIER_LUSTRE},

// Nodes=8
// Nodes=16
{.min_total_bytes = 1024 * 1024 * 5, .max_total_bytes = 1024 * 1024 * 20, .min_nodes = 5, .max_nodes = 20, .min_tasks_per_node = 0, .max_tasks_per_node = 64, .tierid = ESDM_TIER_LUSTRE},


// 128 MiB /////////
// Nodes=1
{.min_total_bytes = 1024 * 1024 * 20, .max_total_bytes = 1024 * 1024 * 500, .min_nodes = 0, .max_nodes = 2, .min_tasks_per_node = 0, .max_tasks_per_node = 3, .tierid = ESDM_TIER_SHM},
{.min_total_bytes = 1024 * 1024 * 20, .max_total_bytes = 1024 * 1024 * 500, .min_nodes = 0, .max_nodes = 2, .min_tasks_per_node = 3, .max_tasks_per_node = 64, .tierid = ESDM_TIER_LUSTRE},

// Nodes=2
// Nodes=4
// Nodes=8
// Nodes=16
{.min_total_bytes = 1024 * 1024 * 20, .max_total_bytes = 1024 * 1024 * 500, .min_nodes = 2, .max_nodes = 20, .min_tasks_per_node = 0, .max_tasks_per_node = 64, .tierid = ESDM_TIER_LUSTRE},
};


char *esdm_suggest_tier(h5sqlite_fapl_t *fapl, int mpi_size, size_t total_bytes) {
  char rank_buf[50];
  sprintf(rank_buf, "%d", fapl->mpi_rank);


  int num_policies = sizeof(policies) / sizeof(ESDM_policy_t);
  printf("[ESDM] Considering %d policies.\n", num_policies);

  // Multifile
  //char* fname = malloc(strlen(fapl->data_fn) + strlen(rank_buf) + 1);
  //strcpy(fname, fapl->data_fn);
  //strcat(fname, rank_buf);


  printf("[ESDM] WARNIGN: do not execute using mpiexec.. this section will try to access SLURM environment variables which will result in a segfault if unpopulated\n");

  int slurm_nodes = 0;
  int slurm_ppn = 0;

  slurm_nodes = atoi(getenv("SLURM_NNODES"));
  slurm_ppn = atoi(getenv("SLURM_TASKS_PER_NODE"));

  printf("[ESDM] KNOWLEDGE: SLURM:  nodes=%d, ppn=%d  \n", slurm_nodes, slurm_ppn);
  printf("[ESDM] KNOWLEDGE: MPI  :  mpi_size=%d  \n", mpi_size);
  printf("[ESDM] KNOWLEDGE: HDF5 :  total_bytes=%d  \n", total_bytes);


  int tierid = 99;
  for (int i = 0; i < num_policies; i++) {
    printf("[ESDM] P%d: size=%d-%d total_bytes=%d\n", i, policies[i].min_total_bytes, policies[i].max_total_bytes, total_bytes);
    if (!(policies[i].min_total_bytes <= total_bytes && total_bytes < policies[i].max_total_bytes)) {
      printf("[ESDM] P%d: size=%d-%d nodes=%d-%d ppn=%d-%d tier=%d - NO (total)\n", i, policies[i].min_total_bytes, policies[i].max_total_bytes, policies[i].min_nodes, policies[i].max_nodes, policies[i].min_tasks_per_node, policies[i].max_tasks_per_node, policies[i].tierid);
      continue;
    }

    if (!(policies[i].min_nodes <= slurm_nodes && slurm_nodes < policies[i].max_nodes)) {
      printf("[ESDM] P%d: size=%d-%d nodes=%d-%d ppn=%d-%d tier=%d - NO (nodes)\n", i, policies[i].min_total_bytes, policies[i].max_total_bytes, policies[i].min_nodes, policies[i].max_nodes, policies[i].min_tasks_per_node, policies[i].max_tasks_per_node, policies[i].tierid);
      continue;
    }

    if (!(policies[i].min_tasks_per_node <= slurm_ppn && slurm_ppn < policies[i].max_tasks_per_node)) {
      printf("[ESDM] P%d: size=%d-%d nodes=%d-%d ppn=%d-%d tier=%d - NO (ppn)\n", i, policies[i].min_total_bytes, policies[i].max_total_bytes, policies[i].min_nodes, policies[i].max_nodes, policies[i].min_tasks_per_node, policies[i].max_tasks_per_node, policies[i].tierid);
      continue;
    }


    printf("[ESDM] P%d: size=%d-%d nodes=%d-%d ppn=%d-%d tier=%d - YES", i, policies[i].min_total_bytes, policies[i].max_total_bytes, policies[i].min_nodes, policies[i].max_nodes, policies[i].min_tasks_per_node, policies[i].max_tasks_per_node, policies[i].tierid);


    // if reached, the policy applies, the remaining policies loose
    tierid = policies[i].tierid;
    break;
  }


  char *tiername = NULL;
  switch (tierid) {
    case ESDM_TIER_SHM:
      tiername = malloc(strlen("/dev/shm/nc_esdmtier") + strlen(rank_buf) + 1);
      strcpy(tiername, "/dev/shm/nc_esdmtier");
      strcat(tiername, rank_buf);
      break;

    case ESDM_TIER_SSD:
      tiername = malloc(strlen("/tmp/nc_esdmtier") + strlen(rank_buf) + 1);
      strcpy(tiername, "/tmp/nc_esdmtier");
      strcat(tiername, rank_buf);
      break;

    case ESDM_TIER_LUSTRE:
    default:
      tiername = malloc(strlen("/mnt/lustre02/work/k20200/k202107/nc_esdmtierfile") + strlen(rank_buf) + 1);
      strcpy(tiername, "/mnt/lustre02/work/k20200/k202107/nc_esdmtierfile");
      strcat(tiername, rank_buf);
  }

  printf("[ESDM] ADAPTIVE TIER SELECTOR: %s\n", tiername);


  return tiername;
}
