##
## Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
## Distributed under the MIT Software License.
## See accompanying file LICENSE.txt or copy at
## https://opensource.org/licenses/MIT
##

ARCH=$(shell arch)
ifeq ($(ARCH),armv7l)
TARGET ?= rpi2
endif
TARGET ?= x86
CC ?= gcc

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
LIBDIR=$(PREFIX)/lib

CFLAGS=-O2 -g -Wall -Werror -Wextra -pedantic -std=c99 -Ilib
LDFLAGS=-O2 -g

LT_CC_DEP:=$(CC)
UNAME:=$(shell uname)

ifeq ($(UNAME),Darwin)
LT_CC:=$(CC)
LT_LD:=ld -dylib
LDFLAGS=
LIB_EXT=dylib
else
LT_CC:=libtool --tag=CC --mode=compile $(CC)
LT_LD:=libtool --tag=CC --mode=link $(CC)
LIB_EXT=lo
endif

MKDIR=mkdir -p

CC=${LT_CC}
CC_DEP=${LT_CC_DEP}
LD=${LT_LD}




HAVEMMX=0
HAVESSE=0
HAVEAVX=0
HAVENEON=0

ifeq ($(TARGET),rpi2)
HAVENEON=1
TARGETCFLAGS=-mfpu=neon
TARGETLDFLAGS=$(TARGETCFLAGS)
endif

ifeq ($(TARGET),x86)
HAVEMMX=1
HAVESSE=1
HAVEAVX=1
TARGETCFLAGS ?= -march=native
ifeq ($(UNAME),Darwin)
TARGETLDFLAGS=-dylib -lc
else
TARGETLDFLAGS=$(TARGETCFLAGS)
endif
endif

LIB_DIR=lib
LIB_DEP_DIR=dep_lib
LIB_OBJ_DIR=obj_lib
SRC_DIR=src
SRC_DEP_DIR=dep_src
SRC_OBJ_DIR=obj_src

ifeq ($(HAVEMMX),1)
MMXFLAGS=-mmmx
SIMDCONFIG+= -DHAVE_MMX
SIMDSRC+= $(LIB_DIR)/sha1_simd_mmx64.c $(LIB_DIR)/ubc_check_simd_mmx64.c
else
SIMDCONFIG+= -DNO_HAVE_MMX
endif

ifeq ($(HAVESSE),1)
SSEFLAGS=-msse -msse2
SIMDCONFIG+= -DHAVE_SSE
SIMDSRC+= $(LIB_DIR)/sha1_simd_sse128.c $(LIB_DIR)/ubc_check_simd_sse128.c
else
SIMDCONFIG+= -DNO_HAVE_SSE
endif

ifeq ($(HAVEAVX),1)
AVXFLAGS=-mavx -mavx2
SIMDCONFIG+= -DHAVE_AVX
SIMDSRC+= $(LIB_DIR)/sha1_simd_avx256.c $(LIB_DIR)/ubc_check_simd_avx256.c
else
SIMDCONFIG+= -DNO_HAVE_AVX
endif

ifeq ($(HAVENEON),1)
NEONFLAGS=-mfpu=neon
SIMDCONFIG+= -DHAVE_NEON
SIMDSRC+= $(LIB_DIR)/sha1_simd_neon128.c $(LIB_DIR)/ubc_check_simd_neon128.c
else
SIMDCONFIG+= -DNO_HAVE_NEON
endif

CFLAGS+= $(SIMDCONFIG) $(TARGETCFLAGS)
LDFLAGS+= $(TARGETLDFLAGS)





FS_LIB=$(filter-out $(wildcard ${LIB_DIR}/*_simd_*.c), $(wildcard ${LIB_DIR}/*.c))
FS_LIB+=$(SIMDSRC)
FS_SRC=$(wildcard ${SRC_DIR}/*.c)
FS_OBJ_LIB=$(FS_LIB:${LIB_DIR}/%.c=${LIB_OBJ_DIR}/%.lo)
FS_OBJ_SRC=$(FS_SRC:${SRC_DIR}/%.c=${SRC_OBJ_DIR}/%.lo)
FS_OBJ=$(FS_OBJ_SRC) $(FS_OBJ_LIB)
FS_DEP_LIB=$(FS_LIB:${LIB_DIR}/%.c=${LIB_DEP_DIR}/%.d)
FS_DEP_SRC=$(FS_SRC:${SRC_DIR}/%.c=${SRC_DEP_DIR}/%.d)
FS_DEP=$(FS_DEP_SRC) $(FS_DEP_LIB)

.SUFFIXES: .c .d

.PHONY: all
all: library tools test

.PHONY: install
install: all
	install bin/sha1dcsum $(BINDIR)
	install bin/sha1dcsum_partialcoll $(BINDIR)
	install bin/libdetectcoll.$(LIB_EXT) $(LIBDIR)

.PHONY: uninstall
uninstall:
	-$(RM) $(BINDIR)/sha1dcsum
	-$(RM) $(BINDIR)/sha1dcsum_partialcoll
	-$(RM) $(LIBDIR)/libdetectcoll.$(LIB_EXT)

.PHONY: clean
clean::
	-find . -type f -name '*.a' -print -delete
	-find . -type f -name '*.d' -print -delete
	-find . -type f -name '*.o' -print -delete
	-find . -type f -name '*.la' -print -delete
	-find . -type f -name '*.lo' -print -delete
	-find . -type f -name '*.so' -print -delete
	-find . -type d -name '.libs' -print | xargs rm -rv
	-rm -rf bin

.PHONY: test
test: tools
	bin/sha1dcsum test/*
	bin/sha1dcsum_partialcoll test/*

.PHONY: tools
tools: sha1dcsum sha1dcsum_partialcoll

.PHONY: sha1dcsum
sha1dcsum: bin/sha1dcsum

.PHONY: sha1dcsum_partialcoll
sha1dcsum_partialcoll: bin/sha1dcsum
	-ln -s sha1dcsum bin/sha1dcsum_partialcoll
	
.PHONY: library
library: bin/libdetectcoll.la

bin/libdetectcoll.la: $(FS_OBJ_LIB)
	${MKDIR} $(shell dirname $@) && ${LD} ${LDFLAGS} $(FS_OBJ_LIB) -o bin/libdetectcoll.$(LIB_EXT)

bin/sha1dcsum: $(FS_OBJ_SRC) library
	${CC} ${CFLAGS} $(FS_OBJ_SRC) $(FS_OBJ_LIB) -Lbin -ldetectcoll -o bin/sha1dcsum 


${SRC_DEP_DIR}/%.d: ${SRC_DIR}/%.c
	${MKDIR} $(shell dirname $@) && $(CC_DEP) $(CFLAGS) -M -MF $@ $<

${SRC_OBJ_DIR}/%.lo ${SRC_OBJ_DIR}/%.o: ${SRC_DIR}/%.c ${SRC_DEP_DIR}/%.d
	${MKDIR} $(shell dirname $@) && $(CC) $(CFLAGS) -o $@ -c $<


${LIB_DEP_DIR}/%mmx64.d: ${LIB_DIR}/%mmx64.c
	${MKDIR} $(shell dirname $@) && $(CC_DEP) $(CFLAGS) $(MMXFLAGS) -M -MF $@ $<

${LIB_DEP_DIR}/%sse128.d: ${LIB_DIR}/%sse128.c
	${MKDIR} $(shell dirname $@) && $(CC_DEP) $(CFLAGS) $(SSEFLAGS) -M -MF $@ $<

${LIB_DEP_DIR}/%avx256.d: ${LIB_DIR}/%avx256.c
	${MKDIR} $(shell dirname $@) && $(CC_DEP) $(CFLAGS) $(AVXFLAGS) -M -MF $@ $<

${LIB_DEP_DIR}/%neon128.d: ${LIB_DIR}/%neon128.c
	${MKDIR} $(shell dirname $@) && $(CC_DEP) $(CFLAGS) $(NEONFLAGS) -M -MF $@ $<

${LIB_DEP_DIR}/%.d: ${LIB_DIR}/%.c
	${MKDIR} $(shell dirname $@) && $(CC_DEP) $(CFLAGS) -M -MF $@ $<



${LIB_OBJ_DIR}/%mmx64.lo ${LIB_OBJ_DIR}/%mmx64.o: ${LIB_DIR}/%mmx64.c ${LIB_DEP_DIR}/%mmx64.d
	${MKDIR} $(shell dirname $@) && $(CC) $(CFLAGS) $(MMXFLAGS) -o $@ -c $<

${LIB_OBJ_DIR}/%sse128.lo ${LIB_OBJ_DIR}/%sse128.o: ${LIB_DIR}/%sse128.c ${LIB_DEP_DIR}/%sse128.d
	${MKDIR} $(shell dirname $@) && $(CC) $(CFLAGS) $(SSEFLAGS) -o $@ -c $<

${LIB_OBJ_DIR}/%avx256.lo ${LIB_OBJ_DIR}/%avx256.o: ${LIB_DIR}/%avx256.c ${LIB_DEP_DIR}/%avx256.d
	${MKDIR} $(shell dirname $@) && $(CC) $(CFLAGS) $(AVXFLAGS) -o $@ -c $<

${LIB_OBJ_DIR}/%neon128.lo ${LIB_OBJ_DIR}/%neon128.o: ${LIB_DIR}/%neon128.c ${LIB_DEP_DIR}/%neon128.d
	${MKDIR} $(shell dirname $@) && $(CC) $(CFLAGS) $(NEONFLAGS) -o $@ -c $<

${LIB_OBJ_DIR}/%.lo ${LIB_OBJ_DIR}/%.o: ${LIB_DIR}/%.c ${LIB_DEP_DIR}/%.d
	${MKDIR} $(shell dirname $@) && $(CC) $(CFLAGS) -o $@ -c $<


-include $(FS_DEP)
