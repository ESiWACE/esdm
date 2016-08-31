// This file is part of h5-memvol.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with h5-memvol.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>

#include <memvol.h>

int main(){

    herr_t status;
    hid_t fprop;
    hid_t fid, group_id, group_id2, group_id3, group_id4;
    hid_t vol_id = H5VL_memvol_init();

    hsize_t dim[2];
    hid_t  dataset_id, datatype_id, dataspace_id;   


    char name[1024];

    fprop = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_vol(fprop, vol_id, &fprop);

    fid = H5Fcreate("test.h5", H5F_ACC_TRUNC, H5P_DEFAULT, fprop);

    H5VLget_plugin_name(fid, name, 1024);
    printf ("Using VOL %s\n", name);
    puts("");

    /* Create a group named "MyGroup" in the file. */
    group_id = H5Gcreate(fid, "MyGroup", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    /* Create a group named "MyGroup1" in "MyGroup". */
    group_id2 = H5Gcreate(group_id, "MyGroup1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    /* Create a group named "MyGroup2" in "MyGroup". */
    group_id3 = H5Gcreate(group_id, "MyGroup2", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    /* Create a group named "MyOtherGroup" in "MyGroup2". */
    group_id4 = H5Gcreate(group_id3, "MyGroup3", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

//---------Dataset create Test--------------------


    dim[0] = 3;
    dim[1] = 4;

    datatype_id = H5Tcopy(H5T_NATIVE_INT);
    

    dataspace_id = H5Screate_simple(2, dim, NULL); 

//Debug Ausgabe
    printf("dataspace_id %zu\n", dataspace_id);
    printf("datatype_id %zu\n", datatype_id);

    dataset_id = H5Dcreate2(fid, "/MyDataset",  datatype_id, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

printf("Test: Creating, dataset_id: %zu \n", dataset_id);

    status = H5Dclose(dataset_id);
    status = H5Sclose(dataspace_id);
    status = H5Tclose(datatype_id);

dataset_id = H5Dopen2(fid, "/MyDataset", H5P_DEFAULT);

printf("Test: Opening, dataset_id: %zu \n", dataset_id);


    status = H5Dclose(dataset_id);
    status = H5Fclose(fid);
//-------------------------------------------------
    H5VL_memvol_finalize();

    return 0;
}
