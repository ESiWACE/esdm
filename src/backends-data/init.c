#include <esdm-internal.h>

#ifdef ESDM_HAS_POSIX
#  include "posix/posix.h"
#  pragma message("Building ESDM with support for generic POSIX backend.")
#endif

#ifdef ESDM_HAS_KDSA
#  include "kdsa/esdm-kdsa.h"
#  pragma message("Building ESDM with Kove XPD KDSA support.")
#endif

#ifdef ESDM_HAS_CLOVIS
#  include "Clovis/clovis.h"
#  pragma message("Building ESDM with Clovis support.")
#endif

#ifdef ESDM_HAS_WOS
#  include "WOS/wos.h"
#  pragma message("Building ESDM with WOS support.")
#endif

esdm_backend_t * esdmI_init_backend(char const * name, esdm_config_backend_t * b){
#ifdef ESDM_HAS_POSIX
  if (strncmp(b->type, "POSIX", 5) == 0) {
    return posix_backend_init(b);
  }
#endif
#ifdef ESDM_HAS_KDSA
  else if (strncasecmp(b->type, "KDSA", 6) == 0) {
    return kdsa_backend_init(b);
  }
#endif
#ifdef ESDM_HAS_CLOVIS
  else if (strncasecmp(b->type, "CLOVIS", 6) == 0) {
    return clovis_backend_init(b);
  }
#endif
#ifdef ESDM_HAS_WOS
  else if (strncmp(b->type, "WOS", 3) == 0) {
    return wos_backend_init(b);
  }
#endif
  return NULL;
}
