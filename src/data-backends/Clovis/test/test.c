#include <stdio.h>

#include "clovis.h"
/*
struct esdm_backend mero_esdm_backend = {
    .eb_magic = 0x3333550033335500,
	.eb_name  = "Mero Clovis",
	.eb_id    = 1234,
	.eb_blocksize = BLOCKSIZE,

	.esdm_backend_init = esdm_backend_mero_init,
	.esdm_backend_fini = esdm_backend_mero_fini,

	.esdm_backend_obj_alloc = esdm_backend_mero_alloc,
	.esdm_backend_obj_open  = esdm_backend_mero_open,
	.esdm_backend_obj_write = esdm_backend_mero_write,
	.esdm_backend_obj_read  = esdm_backend_mero_read,
	.esdm_backend_obj_close = esdm_backend_mero_close
};
*/
int main(int argc, char* argv[])
{
                //"local_addr ha_addr profile process_fid"
    char conf[] = "172.16.154.130@tcp:12345:33:103 172.16.154.130@tcp:12345:34:1 <0x7000000000000001:0> <0x7200000000000001:64>";

    struct esdm_backend_generic *eb;
    int rc;

    printf("Test Mero/Clovis data-backend for ESDM\n");

    printf("mero_esdm_backend name = %s, bs = %llu\n",
            mero_esdm_backend.eb_name,
            (unsigned long long)mero_esdm_backend.eb_blocksize);


    rc = mero_esdm_backend.esdm_backend_init(conf, &eb);
    if (rc != 0) {
        printf("mero_esdm_backend init failed rc=%d\n", rc);
        return rc;
    }

    printf("Clovis connection succeeded\n");
    rc = mero_esdm_backend.esdm_backend_fini(eb);
    if (rc != 0) {
        printf("mero_esdm_backend fini failed rc=%d\n", rc);
        return rc;
    }

    printf("Clovis disconnection succeeded\n");
    return 0;
}
