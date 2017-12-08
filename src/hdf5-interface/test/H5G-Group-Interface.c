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

#define FILE "file-test.h5"

int main()
{
	hid_t fprop;
	hid_t vol_id = H5VLregister_by_name("h5-esdm");

	hid_t       file_id, group_id;  /* identifiers */
	herr_t      status;

	char name[1024];

	// SET VOL PLUGIN /////////////////////////////////////////////////////////
	fprop = H5Pcreate(H5P_FILE_ACCESS);
	H5Pset_vol(fprop, vol_id, &fprop);


	// TODO
	// MOCK SETUP /////////////////////////////////////////////////////////////


	/* Create a new file using default properties. */
	file_id = H5Fcreate(FILE, H5F_ACC_TRUNC, H5P_DEFAULT, fprop);

	/* Create a group named "/MyGroup" in the file. */
	group_id = H5Gcreate2(file_id, "/MyGroup", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	/* Close the group. */
	status = H5Gclose(group_id);

	/* Terminate access to the file. */
	status = H5Fclose(file_id);

	// MOCK CLEANUP ///////////////////////////////////////////////////////////


	// CREATE /////////////////////////////////////////////////////////////////
	// OPEN ///////////////////////////////////////////////////////////////////
	// CLOSE //////////////////////////////////////////////////////////////////
	// READ ///////////////////////////////////////////////////////////////////
	// WRITE //////////////////////////////////////////////////////////////////
	// GET ////////////////////////////////////////////////////////////////////
	// SPECIFIC ///////////////////////////////////////////////////////////////
	// OPTIONAL ///////////////////////////////////////////////////////////////


	// Clean up ///////////////////////////////////////////////////////////////
	H5VLunregister(vol_id);

	return 0;
}
