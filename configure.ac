AC_PREREQ([2.68])
AC_INIT([sha1collisiondetection], [1.0.4], [marc@marc-stevens.nl],[sha1collisiondetection],[https://github.com/cr-marcstevens/sha1collisiondetection])
AC_CONFIG_SRCDIR([README.md])
AC_CONFIG_HEADERS([.config.h])
AX_PREFIX_CONFIG_H([lib/.config.h],[SHA1DC])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

# The version of the libtool library is of the form current:revision:age
#
# See http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
#
# When doing a release, they should be updated like this:
# 1. If no interfaces changed, only implementations: just increment
#    revision.
# 2. If interfaces were added, none removed: increment current, set
#    revision to zero and increment age.
# 3. If interfaces were removed (breaks backward compatibility): increment
#    current, and set both revision and age to zero.

SHA1DC_LT_CURRENT=2
SHA1DC_LT_REVISION=0
SHA1DC_LT_AGE=0


DEFAULT_CFLAGS="-O2 -Wall -Werror -Wextra -Wpedantic -std=c90"
CFLAGS=${CFLAGS:-$DEFAULT_CFLAGS}
MMX64FLAGS=${MMX64FLAGS:-"-mmmx"}
SSE128FLAGS=${SSE128FLAGS:-"-msse -msse2"}
AVX256FLAGS=${AVX256FLAGS:-"-mavx -mavx2"}
AVX512FLAGS=${AVX512FLAGS:-"-mavx512dq"}
NEON128FLAGS=${NEON128FLAGS:-"-mfpu=neon"}



LT_INIT
AC_PROG_LN_S
AC_PROG_MKDIR_P
AC_PROG_AWK
AC_PROG_GREP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CC_C89
AC_PROG_CPP
AC_LANG([C])
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

AC_C_BIGENDIAN(
	[AC_DEFINE(HAVE_BIGENDIAN,1,[Define if machine is big endian])],
	[AC_DEFINE(HAVE_LITTLEENDIAN,1,[Define if machine is little endian])],
	[AC_DEFINE(HAVE_UNKNOWNENDIAN,1,[Define if machine endianness is unknown])])

AX_CHECK_ALIGNED_ACCESS_REQUIRED



AC_SUBST(SHA1DC_LT_CURRENT)
AC_SUBST(SHA1DC_LT_REVISION)
AC_SUBST(SHA1DC_LT_AGE)

SHA1DC_MAJOR_VERSION=`echo AC_PACKAGE_VERSION | $AWK -F. '{print $1}'`
SHA1DC_MINOR_VERSION=`echo AC_PACKAGE_VERSION | $AWK -F. '{print $2}'`
SHA1DC_MICRO_VERSION=`echo AC_PACKAGE_VERSION | $AWK -F. '{print $3}'`
SHA1DC_VERSION=$SHA1DC_MAJOR_VERSION.$SHA1DC_MINOR_VERSION.$SHA1DC_MICRO_VERSION

AC_DEFINE_UNQUOTED([SHA1DC_MAJOR_VERSION],[$SHA1DC_MAJOR_VERSION],[major version])
AC_DEFINE_UNQUOTED([SHA1DC_MINOR_VERSION],[$SHA1DC_MINOR_VERSION],[minor version])
AC_DEFINE_UNQUOTED([SHA1DC_MICRO_VERSION],[$SHA1DC_MICRO_VERSION],[micro version])
AC_DEFINE_UNQUOTED([SHA1DC_VERSION],[$SHA1DC_VERSION],[full version])



AC_ARG_ENABLE([simd], 
	AS_HELP_STRING([--enable-simd[=auto|no|mmx64,sse128,avx256,avx512,neon128]], 
		[Specify list of SIMD versions to compile or use 'no' for none or 
		 'auto' (default) to automatically detect which are supported by the compiler.]),
	[want_simd="$enableval"], [want_simd="auto"])
want_mmx64="no"
want_sse128="no"
want_avx256="no"
want_avx512="no"
want_neon128="no"
AS_IF([echo "$want_simd" | $GREP "auto" >/dev/null],[
	want_mmx64="maybe"
	want_sse128="maybe"
	want_avx256="maybe"
	want_avx512="maybe"
	want_neon128="maybe"
	])
AS_IF([echo "$want_simd" | $GREP "mmx64" >/dev/null], [want_mmx64="yes"])
AS_IF([echo "$want_simd" | $GREP "sse128" >/dev/null], [want_sse128="yes"])
AS_IF([echo "$want_simd" | $GREP "avx256" >/dev/null], [want_avx256="yes"])
AS_IF([echo "$want_simd" | $GREP "avx512" >/dev/null], [want_avx512="yes"])
AS_IF([echo "$want_simd" | $GREP "neon128" >/dev/null], [want_neon128="yes"])

AS_IF([test "$want_mmx64" != "no"], [AX_CHECK_COMPILE_FLAG([$MMX64FLAGS], [have_mmx64=yes], [],
	[-DSHA1DC_HAVE_MMX64 -DTEST_MMX64],[AC_LANG_SOURCE([[#include "lib/simd/simd_test.cinc"]])])])
AS_IF([test "$want_sse128" != "no"], [AX_CHECK_COMPILE_FLAG([$SSE128FLAGS], [have_sse128=yes], [],
	[-DSHA1DC_HAVE_SSE128 -DTEST_SSE128],[AC_LANG_SOURCE([[#include "lib/simd/simd_test.cinc"]])])])
AS_IF([test "$want_avx256" != "no"], [AX_CHECK_COMPILE_FLAG([$AVX256FLAGS], [have_avx256=yes], [],
	[-DSHA1DC_HAVE_AVX256 -DTEST_AVX256],[AC_LANG_SOURCE([[#include "lib/simd/simd_test.cinc"]])])])
AS_IF([test "$want_avx512" != "no"], [AX_CHECK_COMPILE_FLAG([$AVX512FLAGS], [have_avx512=yes], [],
	[-DSHA1DC_HAVE_AVX512 -DTEST_AVX512],[AC_LANG_SOURCE([[#include "lib/simd/simd_test.cinc"]])])])
AS_IF([test "$want_neon128" != "no"], [AX_CHECK_COMPILE_FLAG([$NEON128FLAGS], [have_neon128=yes], [],
	[-DSHA1DC_HAVE_NEON128 -DTEST_NEON128],[AC_LANG_SOURCE([[#include "lib/simd/simd_test.cinc"]])])])

AS_IF([test "$have_mmx64" = "yes"],[AC_DEFINE(HAVE_MMX64,1,[Define if compiler supports MMX instructions])])
AS_IF([test "$have_sse128" = "yes"],[AC_DEFINE(HAVE_SSE128,1,[Define if compiler supports SSE2 instructions])])
AS_IF([test "$have_avx256" = "yes"],[AC_DEFINE(HAVE_AVX256,1,[Define if compiler supports AVX2 instructions])])
AS_IF([test "$have_avx512" = "yes"],[AC_DEFINE(HAVE_AVX512,1,[Define if compiler supports AVX512 instructions])])
AS_IF([test "$have_neon128" = "yes"],[AC_DEFINE(HAVE_NEON128,1,[Define if compiler supports ARM NEON instructions])])
AS_IF([echo "$have_mmx64 $have_sse128 $have_avx256 $have_avx512 $have_neon128" | $GREP "yes" >/dev/null],
	[AC_DEFINE(HAVE_SIMD,1,[Define if any SIMD version is supported])])
AM_CONDITIONAL(HAVE_MMX64, test "x${have_mmx64}" = "xyes")
AM_CONDITIONAL(HAVE_SSE128, test "x${have_sse128}" = "xyes")
AM_CONDITIONAL(HAVE_AVX256, test "x${have_avx256}" = "xyes")
AM_CONDITIONAL(HAVE_AVX512, test "x${have_avx512}" = "xyes")
AM_CONDITIONAL(HAVE_NEON128, test "x${have_neon128}" = "xyes")
AM_CONDITIONAL(HAVE_SIMD, echo "${have_mmx64} ${have_sse128} ${have_avx256} ${have_avx512} ${have_neon128}" | $GREP "yes" >/dev/null)
AC_SUBST([MMX64FLAGS])
AC_SUBST([SSE128FLAGS])
AC_SUBST([AVX256FLAGS])
AC_SUBST([AVX512FLAGS])
AC_SUBST([NEON128FLAGS])



AC_CONFIG_FILES([
 Makefile
])

AC_OUTPUT
