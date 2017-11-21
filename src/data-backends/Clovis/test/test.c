#include <stdio.h>
#include <stdlib.h>

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
    char *object_id = NULL;
    char *object_meta = NULL;
    void *object_handle = NULL;

    char data_w[4096*4];
    char data_r[4096*4];
    int i;
    int rc;

    for (i = 0; i < 4; i++) {
        memset(data_w + 4096 * i, 'a' + i, 4096);
        memset(data_r + 4096 * i, 0      , 4096);
    }
    data_w[4096*4 - 1] = 0;
    data_r[4096*4 - 1] = 0;

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


    rc = mero_esdm_backend.esdm_backend_obj_alloc(eb, 0, NULL, 0, NULL, NULL, &object_id, &object_meta);
    if (rc != 0) {
        printf("mero_esdm_backend alloc failed rc=%d\n", rc);
        goto fini;
    }
    printf("Clovis object allocated: %s\n", object_id);

    rc = mero_esdm_backend.esdm_backend_obj_open(eb, object_id, &object_handle);
    if (rc != 0) {
        printf("mero_esdm_backend open failed rc=%d\n", rc);
        goto fini;
    }
    printf("Clovis object opened: %s\n", object_id);

    rc = mero_esdm_backend.esdm_backend_obj_write(eb, object_handle, 0, 4096 * 4, data_w);
    if (rc != 0) {
        printf("mero_esdm_backend write failed rc=%d\n", rc);
        goto close;
    }
    printf("Clovis object write: %s\n", object_id);

    rc = mero_esdm_backend.esdm_backend_obj_read(eb, object_handle, 0, 4096 * 4, data_r);
    if (rc != 0) {
        printf("mero_esdm_backend read failed rc=%d\n", rc);
        goto close;
    }
    printf("Clovis object read: %s\n", object_id);

    if (memcmp(data_w, data_r, 4096 * 4) != 0) {
        printf("mero_esdm_backend write & read verification failed\n");
        printf("write=%s, read=%s\n", data_w, data_r);
    } else {
        printf("mero_esdm_backend write & read verification succeeded\n");
    }

close:
    rc = mero_esdm_backend.esdm_backend_obj_close(eb, object_handle);
    if (rc != 0) {
        printf("mero_esdm_backend close failed rc=%d\n", rc);
        goto fini;
    }
    printf("Clovis object closed: %s\n", object_id);

fini:
    free(object_id); /* free(NULL) is OK. */
    free(object_meta);

    rc = mero_esdm_backend.esdm_backend_fini(eb);
    if (rc != 0) {
        printf("mero_esdm_backend fini failed rc=%d\n", rc);
        return rc;
    }

    printf("Clovis disconnection succeeded\n");
    return 0;
}
