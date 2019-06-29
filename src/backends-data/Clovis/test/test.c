#include <stdio.h>
#include <stdlib.h>

#include "../clovis.h"
#include "../clovis_internal.h"

int main(int argc, char* argv[])
{
                //"local_addr ha_addr profile process_fid"
    char conf[] = "192.168.168.144@tcp:12345:33:103 192.168.168.144@tcp:12345:34:1 <0x7000000000000001:0> <0x7200000000000001:64>";
    esdm_backend_t *eb = &esdm_backend_t_clovis.ebm_base;
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

    printf("bs = %llu\n", (unsigned long long)eb->blocksize);


    rc = esdm_backend_t_clovis.ebm_ops.esdm_backend_t_init(conf, eb);
    if (rc != 0) {
        printf("esdm_backend_t_clovis.ebm_ops init failed rc=%d\n", rc);
        return rc;
    }
    printf("Clovis connection succeeded\n");


    rc = esdm_backend_t_clovis.ebm_ops.esdm_backend_t_obj_alloc(eb, 0, NULL, 0, NULL, NULL, &object_id, &object_meta);
    if (rc != 0) {
        printf("esdm_backend_t_clovis.ebm_ops alloc failed rc=%d\n", rc);
        goto fini;
    }
    printf("Clovis object allocated: %s\n", object_id);

    rc = esdm_backend_t_clovis.ebm_ops.esdm_backend_t_obj_open(eb, object_id, &object_handle);
    if (rc != 0) {
        printf("esdm_backend_t_clovis.ebm_ops open failed rc=%d\n", rc);
        goto fini;
    }
    printf("Clovis object opened: %s\n", object_id);

    rc = esdm_backend_t_clovis.ebm_ops.esdm_backend_t_obj_write(eb, object_handle, 0, 4096 * 4, data_w);
    if (rc != 0) {
        printf("esdm_backend_t_clovis.ebm_ops write failed rc=%d\n", rc);
        goto close;
    }
    printf("Clovis object write: %s\n", object_id);

    rc = esdm_backend_t_clovis.ebm_ops.esdm_backend_t_obj_read(eb, object_handle, 0, 4096 * 4, data_r);
    if (rc != 0) {
        printf("esdm_backend_t_clovis.ebm_ops read failed rc=%d\n", rc);
        goto close;
    }
    printf("Clovis object read: %s\n", object_id);

    if (memcmp(data_w, data_r, 4096 * 4) != 0) {
        printf("esdm_backend_t_clovis.ebm_ops write & read verification failed\n");
        printf("write=%s, read=%s\n", data_w, data_r);
    } else {
        printf("esdm_backend_t_clovis.ebm_ops write & read verification succeeded\n");
    }

close:
    rc = esdm_backend_t_clovis.ebm_ops.esdm_backend_t_obj_close(eb, object_handle);
    if (rc != 0) {
        printf("esdm_backend_t_clovis.ebm_ops close failed rc=%d\n", rc);
        goto fini;
    }
    printf("Clovis object closed: %s\n", object_id);

    /* index op tests. */
    const char *name = "This is one of my fragments";
    const char *oid  = "<12345678:90aacdef>";

    printf("Clovis mapping insert: '%s' --> '%s'\n", name, oid);
    rc = esdm_backend_t_clovis.ebm_ops.mapping_insert(eb, name, oid);
    if (rc != 0) {
        printf("esdm_backend_t_clovis.ebm_ops mapping_insert failed rc=%d\n", rc);
        goto fini;
    }
    printf("Clovis mapping insert done\n");

    char *new_oid = NULL;
    rc = esdm_backend_t_clovis.ebm_ops.mapping_get(eb, name, &new_oid);
    if (rc != 0) {
        printf("esdm_backend_t_clovis.ebm_ops mapping_get failed rc=%d\n", rc);
        goto fini;
    }

    printf("Clovis mapping got: '%s' --> '%s'\n", name, new_oid);
    free(new_oid);

    /* try to find non-existent mapping */
    new_oid = NULL;
    const char *newname = "This my fragments";
    rc = esdm_backend_t_clovis.ebm_ops.mapping_get(eb, newname, &new_oid);
    if (rc == 0) {
        printf("Failure expected, but something wrong happened = %s\n", new_oid);
        goto fini;
    }
    printf("Clovis find non-existent mapping passed\n");

fini:
    free(object_id); /* free(NULL) is OK. */
    free(object_meta);

    printf("Clovis is about to fini eb = %p\n", eb);
    rc = esdm_backend_t_clovis.ebm_ops.esdm_backend_t_fini(eb);
    if (rc != 0) {
        printf("esdm_backend_t_clovis.ebm_ops fini failed rc=%d\n", rc);
        return rc;
    }

    printf("Clovis disconnection succeeded\n");
    return 0;
}
