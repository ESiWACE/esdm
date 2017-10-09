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

/*
 * This test uses the ESDM high-level API to actually write a contiuous ND subset of a data set
 */
#include <esdm.h>

int main(){
  ESDM_status_t ret;

  // offset in the actual ND dimensions
  uint64_t offset[2] = {0, 0};
  // the size of the data to write
  uint64_t size[2] = {10, 20};
  // data to write
  void * uint64_t = (uint64_t *) malloc(10*20*sizeof(uint64_t));
  for(int x=0; x < 10; x++){
    for(int y=0; y < 20; y++){
      buff[y*10+x] = (y+1)*10 + x + 1;
    }
  }

  // TODO: locate dataset metadata... We assume here the dataset is an uint64_t dataset

  // Write the data to the dataset
  ret = esdm_write(buff, dataset, 2, size, offset);

  // TODO read the buffer
  // TODO compare the results

  free(buff);
  return 0;
}
