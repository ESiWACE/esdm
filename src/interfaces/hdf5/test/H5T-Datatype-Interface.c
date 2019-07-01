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

#include <assert.h>
#include <hdf5.h>
#include <stdio.h>

//#define H5FILE_NAME   "SDScompound.h5"
#define DATASETNAME "ArrayOfStructures"
#define LENGTH 10
#define RANK 1

int main() {
  char *filename = "file-compound-type.h5";

  hid_t fprop;
  hid_t vol_id = H5VLregister_by_name("h5-esdm");

  /* First structure  and dataset*/
  typedef struct s1_t {
    int a;
    float b;
    double c;
  } s1_t;
  s1_t s1[LENGTH];
  hid_t s1_tid; /* File type identifier */

  /* Second structure (subset of s1_t)  and dataset*/
  typedef struct s2_t {
    double c;
    int a;
  } s2_t;
  s2_t s2[LENGTH];
  hid_t s2_tid; /* Memory type handle */

  /* Third "structure" ( will be used to read float field of s1) */
  hid_t s3_tid; /* Memory type handle */
  float s3[LENGTH];

  int i;
  hid_t file, dataset, space; /* Handles */
  herr_t status;
  hsize_t dim[] = {LENGTH}; /* Dataspace dims */

  char name[1024];

  fprop = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_vol(fprop, vol_id, &fprop);

  // TODO
  // MOCK SETUP /////////////////////////////////////////////////////////////

  /*
     * Initialize the data
     */
  for (i = 0; i < LENGTH; i++) {
    s1[i].a = i;
    s1[i].b = i * i;
    s1[i].c = 1. / (i + 1);
  }

  /*
     * Create the data space.
     */
  space = H5Screate_simple(RANK, dim, NULL);

  /*
     * Create the file.
     */
  file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fprop);

  /*
     * Create the memory data type.
     */
  s1_tid = H5Tcreate(H5T_COMPOUND, sizeof(s1_t));
  H5Tinsert(s1_tid, "a_name", HOFFSET(s1_t, a), H5T_NATIVE_INT);
  H5Tinsert(s1_tid, "c_name", HOFFSET(s1_t, c), H5T_NATIVE_DOUBLE);
  H5Tinsert(s1_tid, "b_name", HOFFSET(s1_t, b), H5T_NATIVE_FLOAT);

  /*
     * Create the dataset.
     */
  dataset = H5Dcreate2(file, DATASETNAME, s1_tid, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  /*
     * Wtite data to the dataset;
     */
  status = H5Dwrite(dataset, s1_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, s1);

  /*
     * Release resources
     */
  H5Tclose(s1_tid);
  H5Sclose(space);
  H5Dclose(dataset);
  H5Fclose(file);

  /*
     * Open the file and the dataset.
     */
  file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

  dataset = H5Dopen2(file, DATASETNAME, H5P_DEFAULT);

  /*
     * Create a data type for s2
     */
  s2_tid = H5Tcreate(H5T_COMPOUND, sizeof(s2_t));

  H5Tinsert(s2_tid, "c_name", HOFFSET(s2_t, c), H5T_NATIVE_DOUBLE);
  H5Tinsert(s2_tid, "a_name", HOFFSET(s2_t, a), H5T_NATIVE_INT);

  /*
     * Read two fields c and a from s1 dataset. Fields in the file
     * are found by their names "c_name" and "a_name".
     */
  status = H5Dread(dataset, s2_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, s2);

  /*
     * Display the fields
     */
  printf("\n");
  printf("Field c : \n");
  for (i = 0; i < LENGTH; i++)
    printf("%.4f ", s2[i].c);
  printf("\n");

  printf("\n");
  printf("Field a : \n");
  for (i = 0; i < LENGTH; i++)
    printf("%d ", s2[i].a);
  printf("\n");

  /*
     * Create a data type for s3.
     */
  s3_tid = H5Tcreate(H5T_COMPOUND, sizeof(float));

  status = H5Tinsert(s3_tid, "b_name", 0, H5T_NATIVE_FLOAT);

  /*
     * Read field b from s1 dataset. Field in the file is found by its name.
     */
  status = H5Dread(dataset, s3_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, s3);

  /*
     * Display the field
     */
  printf("\n");
  printf("Field b : \n");
  for (i = 0; i < LENGTH; i++)
    printf("%.4f ", s3[i]);
  printf("\n");

  /*
     * Release resources
     */
  H5Tclose(s2_tid);
  H5Tclose(s3_tid);
  H5Dclose(dataset);
  H5Fclose(file);

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
