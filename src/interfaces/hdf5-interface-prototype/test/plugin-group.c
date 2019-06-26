// This file is part of h5-memvol.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with h5-memvol.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <hdf5.h>

#define FILE "group-test.h5"

int main()
{	
	herr_t	status;
	hid_t   fprop;
	hid_t   fid;
	hid_t   vol_id = H5VLregister_by_name("h5-memvol");

	hid_t g1, g2;
	hid_t plist;

	char name[1024];

	// set VOL plugin
	fprop = H5Pcreate(H5P_FILE_ACCESS);
	H5Pset_vol(fprop, vol_id, &fprop);


	// Bootstrap //////////////////////////////////////////////////////////////
	fid = H5Fcreate(FILE, H5F_ACC_TRUNC, H5P_DEFAULT, fprop);

	// check if correct vol plugin is used
	H5VLget_plugin_name(fid, name, 1024);
	printf ("VOL plugin in use: %s\n", name);


	// CREATE /////////////////////////////////////////////////////////////////
	g1 = H5Gcreate2(fid, "g1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	status = H5Gclose(g1);

	g2 = H5Gcreate2(fid, "g2", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	g1 = H5Gcreate2(g2, "g1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	status = H5Gclose(g1);
	H5Gclose(g2);

	// is this allowed? 
	//  '-> No, a group may have only one child with a certain name (regardless of object type: group, dateset, datatype)
	//g3 = H5Gcreate2(fid, "g1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	//H5Gclose(g3);



	// GET ////////////////////////////////////////////////////////////////////
	printf("Testing additional functions\n");
	g1 = H5Gopen2(fid, "g1", H5P_DEFAULT );
	plist = H5Gget_create_plist(g1);
	H5G_info_t group_info;
	H5Gget_info(g1, & group_info);

	H5Gget_info_by_idx(fid, "g1",  H5_INDEX_CRT_ORDER,  H5_ITER_NATIVE, 0, & group_info, H5P_DEFAULT);
	H5Gget_info_by_idx(fid, "g1",  H5_INDEX_NAME,  H5_ITER_NATIVE, 0, & group_info, H5P_DEFAULT);
	H5Gget_info_by_name(fid, "g1", & group_info, H5P_DEFAULT);
	H5Pclose(plist);


	// NESTING? ///////////////////////////////////////////////////////////////
	H5Gclose(g1);
	g1 = H5Gopen2(fid, "g2", H5P_DEFAULT);
	H5Gclose(g1);
	//g1 = H5Gopen2(fid, "INVALID", H5P_DEFAULT );
	//H5Gclose(g1);

	g1 =  H5Gcreate_anon(fid, H5P_DEFAULT, H5P_DEFAULT);
	H5Gclose(g1);


	// Clean up ///////////////////////////////////////////////////////////////
	H5Fclose(fid);

	// end hdf5 as usual
	H5VLunregister(vol_id);


	printf("Status: %d\n", status);

	return 0;
}
