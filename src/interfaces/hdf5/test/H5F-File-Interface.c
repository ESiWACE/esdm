/* This file is part of ESDM.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief Test for HDF5 file interface on top of ESDM.
 */

#include <assert.h>
#include <hdf5.h>
#include <stdio.h>

int main() {
  char *filename = "file-test.h5";

  hid_t fprop;
  hid_t vol_id = H5VLregister_by_name("h5-esdm");

  hid_t file_id, group_id, dataset_id, dataspace_id, attribute_id;
  herr_t status;

  char plugin_name[1024];

  // SET VOL PLUGIN /////////////////////////////////////////////////////////
  fprop = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_vol(fprop, vol_id, &fprop);

  // MOCK SETUP /////////////////////////////////////////////////////////////
  // MOCK CLEANUP ///////////////////////////////////////////////////////////

  // CREATE /////////////////////////////////////////////////////////////////
  file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fprop);

  // check if VOL in use
  H5VLget_plugin_name(file_id, plugin_name, 1024);
  printf("VOL plugin in use: %s\n", plugin_name);
  // TODO: assert

  // OPEN ///////////////////////////////////////////////////////////////////
  hid_t file_id2;
  file_id2 = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);

  // check if VOL in use
  H5VLget_plugin_name(file_id, plugin_name, 1024);
  printf("VOL plugin in use: %s\n", plugin_name);
  // TODO: assert

  // CLOSE //////////////////////////////////////////////////////////////////
  status = H5Fclose(file_id);

  // READ ///////////////////////////////////////////////////////////////////
  // WRITE //////////////////////////////////////////////////////////////////
  // GET ////////////////////////////////////////////////////////////////////
  // SPECIFIC ///////////////////////////////////////////////////////////////
  // OPTIONAL ///////////////////////////////////////////////////////////////

  // CLOSE //////////////////////////////////////////////////////////////////

  // Clean up ///////////////////////////////////////////////////////////////
  //H5VLunregister(vol_id);

  return 0;
}
