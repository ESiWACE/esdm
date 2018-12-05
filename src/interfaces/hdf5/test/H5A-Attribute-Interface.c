/* This file is part of ESDM.
 *
 * This program is is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * @file
 * @brief Test for HDF5 attribute interface on top of ESDM.
 */

#include <stdio.h>
#include <hdf5.h>

#include <assert.h>


int main()
{
	hid_t fprop;
	hid_t vol_id = H5VLregister_by_name("h5-esdm");

	hid_t file_id, group_id, dataset_id, dataspace_id, attribute_id;
	herr_t status;

	hsize_t     dims;
	int         attr_data[2];



	char* filename = "file-test.h5";



	// SET VOL PLUGIN /////////////////////////////////////////////////////////
	fprop = H5Pcreate(H5P_FILE_ACCESS);
	H5Pset_vol(fprop, vol_id, &fprop);


	// MOCK SETUP /////////////////////////////////////////////////////////////
	/* Initialize the attribute data. */
	attr_data[0] = 100;
	attr_data[1] = 200;


	/* Open an existing file. */
	file_id = H5Fopen(filename, H5F_ACC_RDWR, fprop);

	/* Open an existing dataset. */
	dataset_id = H5Dopen2(file_id, "/dset", H5P_DEFAULT);

	/* Create the data space for the attribute. */
	dims = 2;
	dataspace_id = H5Screate_simple(1, &dims, NULL);



	// CREATE /////////////////////////////////////////////////////////////////
	/* Create a dataset attribute. */
	attribute_id = H5Acreate2 (dataset_id, "Units", H5T_STD_I32BE, dataspace_id, 
			H5P_DEFAULT, H5P_DEFAULT);

	/* Write the attribute data. */
	status = H5Awrite(attribute_id, H5T_NATIVE_INT, attr_data);

	
	
	// CLOSE //////////////////////////////////////////////////////////////////
	/* Close the attribute. */
	status = H5Aclose(attribute_id);




	// TODO
	// OPEN ///////////////////////////////////////////////////////////////////
	// CLOSE //////////////////////////////////////////////////////////////////
	// READ ///////////////////////////////////////////////////////////////////
	// WRITE //////////////////////////////////////////////////////////////////
	// GET ////////////////////////////////////////////////////////////////////
	// SPECIFIC ///////////////////////////////////////////////////////////////
	// OPTIONAL ///////////////////////////////////////////////////////////////



	// MOCK CLEANUP ///////////////////////////////////////////////////////////
	/* Close the dataspace. */
	status = H5Sclose(dataspace_id);

	/* Close to the dataset. */
	status = H5Dclose(dataset_id);

	/* Close the file. */
	status = H5Fclose(file_id);


	// Clean up ///////////////////////////////////////////////////////////////
	H5VLunregister(vol_id);

	return 0;
}
