#include <stdio.h>
#include <time.h>
#include <memvol.h>

int main(int argc, char** argv) {

    /* setup */
    clock_t begin, end;
    double delta_create = 0.0;
    double delta_write = 0.0;
    double delta_read= 0.0;
    double delta_close = 0.0;
    hid_t fid, fprop, vol_id;
    fprop = H5Pcreate(H5P_FILE_ACCESS);
    vol_id = H5VL_memvol_init();
    H5Pset_vol(fprop, vol_id, &fprop);

 	const int nloops = 10000;
    
    int i;
    for (i = 0; i < nloops; i++) {
        char name[12];
        sprintf(name, "test%d.h5", i);

        hsize_t dims[2] = {3, 4};	
        int data[3][4];
        int counter = 0;
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 4; y++) {
                data[x][y] = 1000 + ++counter;
            }
        }

        hid_t space = H5Screate_simple(2, dims, NULL);
        begin = clock();
        fid = H5Fcreate(name, H5F_ACC_TRUNC, H5P_DEFAULT, fprop);
        end = clock();
        delta_create += (double)(end-begin) / CLOCKS_PER_SEC;

        hid_t did = H5Dcreate(fid, "test", H5T_NATIVE_INT, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);	

        begin = clock();
        H5Dwrite(did, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
        end = clock();
        delta_write += (double)(end-begin) / CLOCKS_PER_SEC;

        begin = clock();
        H5Dclose(did);
        end = clock();
        delta_close += (double)(end-begin) / CLOCKS_PER_SEC;
    }

    /* calculate means */
    double avg_create = (double)delta_create / nloops;
    printf("Average Execution Time for H5Fcreate: %.16f\n", avg_create);
    double avg_write = (double)delta_write / nloops;
    printf("Average Execution Time for H5Dwrite: %.16f\n", avg_write);
    double avg_close = (double)delta_close / nloops;
    printf("Average Execution Time for H5Fclose: %.16f\n", avg_close);
     
	H5VL_memvol_finalize();
}
