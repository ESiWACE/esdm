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
    hid_t fprop;
    hid_t fid, group_id, group_id2;
    hid_t vol_id = H5VL_memvol_init();

    char name[1024];

    fprop = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_vol(fprop, vol_id, &fprop);

    fid = H5Fcreate("test.h5", H5F_ACC_TRUNC, H5P_DEFAULT, fprop);
    H5VLget_plugin_name(fid, name, 1024);
    printf ("Using VOL %s\n", name);

    /* Create a group named "/MyGroup" in the file. */
    group_id = H5Gcreate(fid, "/MyGroup", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    /* Create a group named "/MyOtherGroup" in "/MyGroup". */
    group_id2 = H5Gcreate(group_id, "/MyOtherGroup", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    H5VL_memvol_finalize();

    return 0;
}
