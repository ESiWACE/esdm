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

#define FILE "dataset-test.h5"

int main()
{
	herr_t	status;
	hid_t	fprop;
	hid_t	file_id, dataset_id, dataspace_id;
	hid_t	vol_id = H5VLregister_by_name("h5-memvol");

	hsize_t	dims[2];
	hid_t	plist;

	char name[1024];


	// Bootstrap //////////////////////////////////////////////////////////////
	// set VOL plugin
	fprop = H5Pcreate(H5P_FILE_ACCESS);
	H5Pset_vol(fprop, vol_id, &fprop);
	
	file_id = H5Fcreate(FILE, H5F_ACC_TRUNC, H5P_DEFAULT, fprop);
  
  	// check if correct VOL plugin is used
	H5VLget_plugin_name(file_id, name, 1024);
	printf ("VOL plugin in use: %s\n", name);


	// CREATE /////////////////////////////////////////////////////////////////
	/* Create the data space for the dataset. */
	dims[0] = 4; 
	dims[1] = 6; 
	dataspace_id = H5Screate_simple(2, dims, NULL);

	/* Create the dataset. */
	dataset_id = H5Dcreate2(file_id, "/dset", H5T_STD_I32BE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	status = H5Dget_offset(dataset_id);


	// CLOSE //////////////////////////////////////////////////////////////////
	status = H5Dclose(dataset_id);
	status = H5Sclose(dataspace_id);


	// OPEN ///////////////////////////////////////////////////////////////////
    dataset_id = H5Dopen2(file_id, "/dset", H5P_DEFAULT);


	// WRITE //////////////////////////////////////////////////////////////////
	int i, j, dset_data[4][6];

	/* Prepare the dataset. */
	printf("BUFFER: ");
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 6; j++) {
			dset_data[i][j] = i * 6 + j + 1;
			printf("%d,", dset_data[i][j]);
		}
	}
	printf("\n");

	/* Write the dataset. */
	status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, dset_data);


	// READ ///////////////////////////////////////////////////////////////////
	int dset_data_read[4][5];

	status = H5Dread(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, dset_data_read);
	
	printf("BUFFER: ");
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 5; j++) {
			printf("%d,", dset_data_read[i][j]);
		}
	}
	printf("\n");


	// Clean up ///////////////////////////////////////////////////////////////
	status = H5Dclose(dataset_id);
	status = H5Fclose(file_id);

	// end hdf5 as usual
	H5VLunregister(vol_id);

	return 0;
}
