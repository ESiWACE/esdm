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

#ifndef __WOS_ERROR_H
#define __WOS_ERROR_H

char wos_error[][50] = {"No node for policy", "no node for object", "unknown policy name", "Internal Error",
"Object frozen", "Invalid id for object", "No Space", "Object not found", "Object Corrupted",
"File System corrupted", "Policy not supported", "Error I/O", "Invalid Object Size", "Missing Object",
"Temporarily Not supported", "Out of Memory", "Reservation not found", "Empty object", "Invalid Metadata key",
"Unused Reservation", "Wire corruption", "Command Timeout"};

#endif
