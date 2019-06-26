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

#include <memvol.h>

typedef struct {
   double re;
   double im;
   char name[10];
   int val;
} complex_type;



int main(){
  hid_t fprop;
  hid_t fid;
  hid_t vol_id = H5VL_memvol_init();
  char name[1024];

  // create some datatypes

  hid_t tid = H5Tcreate (H5T_COMPOUND, sizeof(complex_type));
  H5Tinsert(tid, "re", HOFFSET(complex_type,re), H5T_NATIVE_DOUBLE);
  H5Tinsert(tid, "im", HOFFSET(complex_type,im), H5T_NATIVE_DOUBLE);
  hid_t s10 = H5Tcopy(H5T_C_S1);
  H5Tset_size(s10, 10);
  H5Tinsert(tid, "name", HOFFSET(complex_type,name), s10);
  H5Tinsert(tid, "val", HOFFSET(complex_type,val), H5T_NATIVE_INT);

  // packed version of the datatype
  hid_t disk_tid = H5Tcopy (tid);
  H5Tpack(disk_tid);

  fprop = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_vol(fprop, vol_id, &fprop);

  fid = H5Fcreate("test", H5F_ACC_TRUNC, H5P_DEFAULT, fprop);
  H5VLget_plugin_name(fid, name, 1024);
  printf ("%s using VOL %s\n", __FILE__ , name);

  assert(H5Tcommit(fid, "t_complex", tid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) >= 0);
  assert(H5Tcommit(fid, "t_complex_p", disk_tid,  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) >= 0);

  hid_t tid_stored1 = H5Topen(fid, "t_complex", H5P_DEFAULT);
  hid_t tid_stored2 = H5Topen(fid, "t_complex_p", H5P_DEFAULT);
  // hid_t tid_stored3 = H5Topen(fid, "NotExisting", H5P_DEFAULT);
  // assert(tid_stored3 < 0);

  assert(H5Tequal(tid_stored1, tid));
  assert(H5Tequal(tid_stored2, disk_tid));

  H5Fclose(fid);

  H5Tclose(tid);
  H5Tclose(disk_tid);

  H5VL_memvol_finalize();

  return 0;
}
