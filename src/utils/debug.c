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
 * @brief Debug adds functionality for logging and inspection of ESDM datatypes
 *        during development.
 *
 */

#include <stdio.h>
#include <stdarg.h>

#include <glib.h>

#include <esdm-internal.h>





void esdm_log(uint32_t loglevel, const char* format, ...)
{
    uint32_t active_loglevel = 99;

	if ( loglevel <= active_loglevel ) {
		va_list args;
    printf("[ESDM] ");
		va_start(args,format);
		vprintf(format,args);
		va_end(args);
	}
}



static void print_hashtable_entry (gpointer key, gpointer value, gpointer user_data)
{
	ESDM_DEBUG_FMT("GHashTable Entry: key=%p (s:%s), value=%p (s:%s), user_data=%p\n", key, key, value, value, user_data);
}

void esdm_print_hashtable (GHashTable * tbl){
  g_hash_table_foreach (tbl, print_hashtable_entry, NULL);
}
