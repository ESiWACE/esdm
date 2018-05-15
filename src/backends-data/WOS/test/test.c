#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../wos.h"

#define NUM_OBJECTS 4

int main(int argc, char *argv[])
{
	char conf[] = "host=192.168.80.33;policy=test;";
	esdm_backend_t *eb = &esdm_backend_wos.ebm_base;
	char *object_id = NULL;
	char *object_meta = NULL;
	void *object_handle = NULL;

	int data_size = eb->blocksize * NUM_OBJECTS;
	char data_w[data_size];
	char data_r[data_size];
	int i;
	int rc;

	for (i = 0; i < NUM_OBJECTS; i++) {
		memset(data_w + eb->blocksize * i, 'a' + i, eb->blocksize);
		memset(data_r + eb->blocksize * i, 0, eb->blocksize);
	}
	data_w[data_size - 1] = 0;
	data_r[data_size - 1] = 0;

	printf("Test wos data-backend for ESDM\n");

	rc = esdm_backend_wos.ebm_ops.esdm_backend_init(conf, eb);
	if (rc != 0) {
		printf("esdm_backend_wos.ebm_ops init failed rc=%d\n", rc);
		return rc;
	}
	printf("wos connection succeeded\n");

	rc = esdm_backend_wos.ebm_ops.esdm_backend_obj_alloc(eb, 1, &data_size, esdm_char, NULL, NULL, &object_id, &object_meta);
	if (rc != 0) {
		printf("esdm_backend_wos.ebm_ops alloc failed rc=%d\n", rc);
		goto fini;
	}
	printf("wos object allocated: %s\n", object_id);

	rc = esdm_backend_wos.ebm_ops.esdm_backend_obj_open(eb, object_id, &object_handle);
	if (rc != 0) {
		printf("esdm_backend_wos.ebm_ops open failed rc=%d\n", rc);
		goto fini;
	}
	printf("wos object opened: %s\n", object_id);

	rc = esdm_backend_wos.ebm_ops.esdm_backend_obj_write(eb, object_handle, 0, data_size, data_w);
	if (rc != 0) {
		printf("esdm_backend_wos.ebm_ops write failed rc=%d\n", rc);
		goto close;
	}
	printf("wos object write: %s\n", object_id);

	rc = esdm_backend_wos.ebm_ops.esdm_backend_obj_read(eb, object_handle, 0, data_size, data_r);
	if (rc != 0) {
		printf("esdm_backend_wos.ebm_ops read failed rc=%d\n", rc);
		goto close;
	}
	printf("wos object read: %s\n", object_id);

	if (memcmp(data_w, data_r, data_size)) {
		printf("esdm_backend_wos.ebm_ops write & read verification failed\n");
		printf("write=%s\nread=%s\n", data_w, data_r);
	} else {
		printf("esdm_backend_wos.ebm_ops write & read verification succeeded\n");
	}

      close:
	rc = esdm_backend_wos.ebm_ops.esdm_backend_obj_close(eb, object_handle);
	if (rc != 0) {
		printf("esdm_backend_wos.ebm_ops close failed rc=%d\n", rc);
		goto fini;
	}
	printf("wos object closed: %s\n", object_id);

      fini:
	if (object_id)
		free(object_id);
	if (object_meta)
		free(object_meta);

	rc = esdm_backend_wos.ebm_ops.esdm_backend_fini(eb);
	if (rc != 0) {
		printf("esdm_backend_wos.ebm_ops fini failed rc=%d\n", rc);
		return rc;
	}

	printf("wos disconnection succeeded\n");
	return 0;
}
