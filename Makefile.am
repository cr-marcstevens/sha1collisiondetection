EXTRA_DIST       = README.md LICENSE.txt
ACLOCAL_AMFLAGS  = -I m4
AM_CPPFLAGS=-I$(top_srcdir)/lib
# prevent parallel runs of simd_table_gen
.NOTPARALLEL:


# install header <sha1dc/sha1.h>
include_sha1cddir=$(includedir)/sha1dc
include_sha1cd_HEADERS=lib/sha1.h


# the main sha1 with collision detection library
lib_LTLIBRARIES=lib/libsha1detectcoll.la

lib_libsha1detectcoll_la_SOURCES=\
	lib/sha1.c lib/sha1.h \
	lib/ubc_check.c lib/ubc_check.h \
	lib/simd/sha1_simd.c \
	lib/simd/sha1_simd_mmx64.c lib/simd/sha1_simd_sse128.c \
	lib/simd/sha1_simd_avx256.c lib/simd/sha1_simd_avx512.c \
	lib/simd/sha1_simd_neon128.c

lib_libsha1detectcoll_la_LDFLAGS=-no-undefined -version-info @SHA1DC_LT_CURRENT@:@SHA1DC_LT_REVISION@:@SHA1DC_LT_AGE@

nodist_lib_libsha1detectcoll_la_SOURCES = lib/simd/dvs_simd.h lib/simd/dvs_simd.c

BUILT_SOURCES = lib/simd/dvs_simd.h lib/simd/dvs_simd.c
CLEANFILES = lib/simd/dvs_simd.h lib/simd/dvs_simd.c

lib/simd/dvs_simd.h lib/simd/dvs_simd.c: Makefile lib/simd/DV_data.txt
	if [ ! -f $@ ] || [ $@ -ot Makefile ]; then bin/simd_table_gen$(EXEEXT) lib/simd/DV_data.txt ; fi

# standard program sha1dcsum and test program sha1dcsum_partialcoll
bin_PROGRAMS=bin/sha1dcsum

EXTRA_PROGRAMS=bin/sha1dcsum_partialcoll bin/simd_table_gen

bin_sha1dcsum_SOURCES=src/main.c
bin_sha1dcsum_LDADD=lib/libsha1detectcoll.la

bin_sha1dcsum_partialcoll_SOURCES=src/main.c
bin_sha1dcsum_partialcoll_LDADD=lib/libsha1detectcoll.la

bin_simd_table_gen_SOURCES=src/simd_table_gen.c


# add proper compiler flags for each simd version if enabled
if HAVE_MMX64
%_mmx64.lo   %_mmx64.o  : CFLAGS += $(MMX64FLAGS)
endif
if HAVE_SSE128
%_sse128.lo  %_sse128.o : CFLAGS += $(SSE128FLAGS)
endif
if HAVE_AVX256
%_avx256.lo  %_avx256.o : CFLAGS += $(AVX256FLAGS)
endif
if HAVE_AVX512
%_avx512.lo  %_avx512.o : CFLAGS += $(AVX512FLAGS)
endif
if HAVE_NEON128
%_neon128.lo %_neon128.o: CFLAGS += $(NEON128FLAGS)
endif
