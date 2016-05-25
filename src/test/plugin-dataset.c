// This file is part of h5-memvol.
//
// This program is is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with h5-memvol.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>

#include <hdf5.h>

int main(){
	hid_t	fprop;
	hid_t	file_id, dataset_id, dataspace_id;
	hid_t	vol_id = H5VLregister_by_name("h5-memvol");
	herr_t	status;

	hsize_t	dims[2];
	hid_t	plist;

	char name[1024];

	fprop = H5Pcreate(H5P_FILE_ACCESS);
	H5Pset_vol(fprop, vol_id, &fprop);

	// hdf5 as usual
	file_id = H5Fcreate("test", H5F_ACC_TRUNC, H5P_DEFAULT, fprop);
	H5VLget_plugin_name(file_id, name, 1024);
	printf ("FAPL set to use VOL %s\n", name);


	/* Create the data space for the dataset. */
	dims[0] = 4; 
	dims[1] = 6; 
	dataspace_id = H5Screate_simple(2, dims, NULL);

	/* Create the dataset. */
	dataset_id = H5Dcreate2(file_id, "/dset", H5T_STD_I32BE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	/* End access to the dataset and release resources used by it. */
	status = H5Dclose(dataset_id);

	/* Terminate access to the data space. */ 
	status = H5Sclose(dataspace_id);

	status = H5Fclose(file_id);

	// end hdf5 as usual
	H5VLunregister(vol_id);

	return 0;
}
