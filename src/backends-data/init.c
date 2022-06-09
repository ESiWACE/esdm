#include <esdm-internal.h>

#include "dummy/dummy.h"


#ifdef ESDM_HAS_S3
#  include "s3/s3.h"
#  pragma message("Building ESDM with support for S3 backend.")
#endif

#ifdef ESDM_HAS_POSIX
#  include "posix/posix.h"
#  include "posixi/posixi.h"
#  pragma message("Building ESDM with support for generic POSIX backend.")
#endif

#ifdef ESDM_HAS_IME
#  include "ime/ime.h"
#  pragma message("Building ESDM with IME support.")
#endif

#ifdef ESDM_HAS_KDSA
#  include "kdsa/esdm-kdsa.h"
#  pragma message("Building ESDM with Kove XPD KDSA support.")
#endif

#ifdef ESDM_HAS_MOTR
#  include "Motr/client.h"
#  pragma message("Building ESDM with Motr support.")
#endif

#ifdef ESDM_HAS_WOS
#  include "WOS/wos.h"
#  pragma message("Building ESDM with WOS support.")
#endif

#ifdef ESDM_HAS_PMEM
#  include "pmem/esdm-pmem.h"
#  pragma message("Building ESDM with PMEM support.")
#endif

esdm_backend_t * esdmI_init_backend(char const * name, esdm_config_backend_t * b){
  if (strncmp(b->type, "DUMMY", 5) == 0) {
    return dummy_backend_init(b);
  }
#ifdef ESDM_HAS_POSIX
  else if (strncmp(b->type, "POSIXI", 6) == 0) {
    return posixi_backend_init(b);
  }
  else if (strncmp(b->type, "POSIX", 5) == 0) {
    return posix_backend_init(b);
  }
#endif
#ifdef ESDM_HAS_IME
  else if (strncasecmp(b->type, "IME", 3) == 0) {
    return ime_backend_init(b);
  }
#endif
#ifdef ESDM_HAS_KDSA
  else if (strncasecmp(b->type, "KDSA", 4) == 0) {
    return kdsa_backend_init(b);
  }
#endif
#ifdef ESDM_HAS_MOTR
  else if (strncasecmp(b->type, "MOTR", 6) == 0) {
    return motr_backend_init(b);
  }
#endif
#ifdef ESDM_HAS_WOS
  else if (strncmp(b->type, "WOS", 3) == 0) {
    return wos_backend_init(b);
  }
#endif
#ifdef ESDM_HAS_PMEM
  else if (strncmp(b->type, "PMEM", 4) == 0) {
    return pmem_backend_init(b);
  }
#endif
#ifdef ESDM_HAS_S3
  else if (strncmp(b->type, "S3", 2) == 0){
    return s3_backend_init(b);
  }
#endif
  return NULL;
}
