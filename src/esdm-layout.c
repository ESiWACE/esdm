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
 * @brief The layout component fragments and reconstructs logical data.
 *
 * This file contains the layout implementation.
 *
 *
 * TODO:
 *
 *	mapper:
 *		1d in => (reorder?) (single/multiple) sequence
 *			index,   blocksize[dim], filling curve
 *
 *
 *		2d in => (reorder?) (single/multiple) sequence
 *			decoupling of indexes?
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>

#include <jansson.h>

#include <esdm.h>
#include <esdm-internal.h>




// layout component may like to have following capabilites:
//
// domain decomposition
//
// adaptive mesh refinements
//		e.g.  refine and coarsen operations on regular grid
//
//
// minimization surface vs volume


// space filling curves
//	hilber curve
//	z-ordering?



// also have a look at device data environments from openmp
// openmp:  map clause





esdm_layout_t* esdm_layout_init(esdm_instance_t* esdm) {
	ESDM_DEBUG(__func__);	

	esdm_layout_t* layout = NULL;
	layout = (esdm_layout_t*) malloc(sizeof(esdm_layout_t));
	return layout;
}


/**
* Shutdown ESDM:
*  - finalize submodules
*  - free data structures
*
* @return status
*/
esdm_status_t esdm_layout_finalize()
{
	ESDM_DEBUG(__func__);	
	return ESDM_SUCCESS;
}




/**
 * Splits pending requests into one or more requests based on performance
 * estimates obtained from available backends.
 *
 */
esdm_status_t esdm_layout_recommendation(esdm_instance_t *esdm, esdm_fragment_t *in, esdm_fragment_t *out)
{
	ESDM_DEBUG(__func__);	

	// pickup the performance estimate for each backend module
	esdm_module_type_array_t * backends;
	esdm_modules_get_by_type(ESDM_TYPE_DATA, backends);
	//int i;
	//for(i=0; i < backends->count; i++){
	//	//esdm_backend_estimate_performance((esdm_backend_t*) backends->module, 1234);
	//}

	// now choice the best module
}




/**
 * The layout reconstructor finds a reconstruction for subspace of a dataset.
 *
 * The reconstruction should take performance considerations into account. 
 */
esdm_fragment_t* esdm_layout_reconstruction(esdm_dataset_t *dataset, esdm_dataspace_t *subspace)
{
	ESDM_DEBUG(__func__);	


	// TODO: consider decision components to choose fragments (also choose from replicas)

	// for fragment in fragments 
	//		if   subspaces_overlap(subspace, fragment)
	//			consider for reconstruction
	
	
	// subspaces_overlap?
	//	overlap = true
	//	while currentdim < totaldims
	//		if 


	


	return ESDM_SUCCESS;
}





esdm_status_t esdm_layout_stat(char* desc)
{
	ESDM_DEBUG(__func__);	
	ESDM_DEBUG("received metadata lookup request");


	// parse text into JSON structure
	json_t *root = load_json(desc);

	if (root) {
		// print and release the JSON structure
		print_json(root);
		json_decref(root);
	}



}

