#include <stdio.h>
#include <stdlib.h>

#include "../clovis.h"

int main(int argc, char* argv[])
{
                //"local_addr ha_addr profile process_fid"
    char conf[] = "172.16.154.130@tcp:12345:33:103 172.16.154.130@tcp:12345:34:1 <0x7000000000000001:0> <0x7200000000000001:64>";
    esdm_backend_t *eb = &esdm_backend_clovis.ebm_base;
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

    printf("bs = %llu\n",
            (unsigned long long)eb->blocksize);


    rc = esdm_backend_clovis.ebm_ops.esdm_backend_init(conf, eb);
    if (rc != 0) {
        printf("esdm_backend_clovis.ebm_ops init failed rc=%d\n", rc);
        return rc;
    }
    printf("Clovis connection succeeded\n");


    rc = esdm_backend_clovis.ebm_ops.esdm_backend_obj_alloc(eb, 0, NULL, 0, NULL, NULL, &object_id, &object_meta);
    if (rc != 0) {
        printf("esdm_backend_clovis.ebm_ops alloc failed rc=%d\n", rc);
        goto fini;
    }
    printf("Clovis object allocated: %s\n", object_id);

    rc = esdm_backend_clovis.ebm_ops.esdm_backend_obj_open(eb, object_id, &object_handle);
    if (rc != 0) {
        printf("esdm_backend_clovis.ebm_ops open failed rc=%d\n", rc);
        goto fini;
    }
    printf("Clovis object opened: %s\n", object_id);

    rc = esdm_backend_clovis.ebm_ops.esdm_backend_obj_write(eb, object_handle, 0, 4096 * 4, data_w);
    if (rc != 0) {
        printf("esdm_backend_clovis.ebm_ops write failed rc=%d\n", rc);
        goto close;
    }
    printf("Clovis object write: %s\n", object_id);

    rc = esdm_backend_clovis.ebm_ops.esdm_backend_obj_read(eb, object_handle, 0, 4096 * 4, data_r);
    if (rc != 0) {
        printf("esdm_backend_clovis.ebm_ops read failed rc=%d\n", rc);
        goto close;
    }
    printf("Clovis object read: %s\n", object_id);

    if (memcmp(data_w, data_r, 4096 * 4) != 0) {
        printf("esdm_backend_clovis.ebm_ops write & read verification failed\n");
        printf("write=%s, read=%s\n", data_w, data_r);
    } else {
        printf("esdm_backend_clovis.ebm_ops write & read verification succeeded\n");
    }

close:
    rc = esdm_backend_clovis.ebm_ops.esdm_backend_obj_close(eb, object_handle);
    if (rc != 0) {
        printf("esdm_backend_clovis.ebm_ops close failed rc=%d\n", rc);
        goto fini;
    }
    printf("Clovis object closed: %s\n", object_id);

fini:
    free(object_id); /* free(NULL) is OK. */
    free(object_meta);

    rc = esdm_backend_clovis.ebm_ops.esdm_backend_fini(eb);
    if (rc != 0) {
        printf("esdm_backend_clovis.ebm_ops fini failed rc=%d\n", rc);
        return rc;
    }

    printf("Clovis disconnection succeeded\n");
    return 0;
}
