/* This file is part of ESDM. See the enclosed LICENSE */

/**
 * @file
 * @brief Entry point for ESDM streaming implementation
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esdm-internal.h>


#ifdef HAVE_SCIL
SCIL_Datatype_t ea_esdm_datatype_to_scil(smd_basic_type_t type){
  switch (type) {
    case (SMD_TYPE_UNKNOWN):
      return SCIL_TYPE_UNKNOWN;
    case (SMD_TYPE_UINT8):
    case (SMD_TYPE_INT8):
      return SCIL_TYPE_INT8;
    case (SMD_TYPE_UINT16):
    case (SMD_TYPE_INT16):
      return SCIL_TYPE_INT16;
    case (SMD_TYPE_UINT32):
    case (SMD_TYPE_INT32):
      return SCIL_TYPE_INT32;
    case (SMD_TYPE_UINT64):
    case (SMD_TYPE_INT64):
      return SCIL_TYPE_INT64;
    case (SMD_TYPE_FLOAT):
      return SCIL_TYPE_FLOAT;
    case (SMD_TYPE_DOUBLE):
      return SCIL_TYPE_DOUBLE;
    case (SMD_TYPE_STRING):
      return SCIL_TYPE_STRING;
    default:
      printf("Unsupported datatype: %d\n", type);
      return 0;
  }
}
#endif
