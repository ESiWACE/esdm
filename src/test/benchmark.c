#include <stdio.h>
#include <time.h>
#include <memvol.h>

int main(int argc, char** argv) {

    /* setup */
    clock_t begin, end;
    double delta_ts = 0.0;
    hid_t fid, fprop, vol_id;
    fprop = H5Pcreate(H5P_FILE_ACCESS);
    vol_id = H5VL_memvol_init();
    H5Pset_vol(fprop, vol_id, &fprop);
    
    int i;
    for (i = 0; i < 10000; i++) {
        char name[12];
        sprintf(name, "test%d.h5", i);

        begin = clock();

        /* measured code */
        fid = H5Fcreate(name, H5F_ACC_TRUNC, H5P_DEFAULT, fprop);

        end = clock();
        delta_ts += (double)(end-begin) / CLOCKS_PER_SEC;

        H5VL_memvol_finalize();
    }

    /* calculate means */
    double avg_t = (double)delta_ts / 10000;
    printf("Average Execution Time for H5Fcreate: %f\n", avg_t);
}
