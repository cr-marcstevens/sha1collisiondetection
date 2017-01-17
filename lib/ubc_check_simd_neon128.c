// this file generates neon 32x4 code using neon MACROS (arm_neon.h) and generic SIMD code (sha1_simd.cinc) 
#ifdef HAVE_NEON
#include "ubc_check.h"
#include "simd_neon128.h"

#define UBC_CHECK_SIMD ubc_check_neon128

#include "ubc_check_simd.cinc"

#else

#pragma message "The file: ubc_check_simd_neon128.c is not compiled for this architecture."

#endif //HAVE_NEON