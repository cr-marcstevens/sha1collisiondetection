##
## Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow (danshu@microsoft.com)
## Distributed under the MIT Software License.
## See accompanying file LICENSE.txt or copy at
## https://opensource.org/licenses/MIT
##

PREFIX ?= /usr/local
BINDIR=$(PREFIX)/bin
LIBDIR=$(PREFIX)/lib

CFLAGS=-O2 -Wall -Werror -Wextra -pedantic -std=c90 -Ilib
LDFLAGS=-O2 

LT_CC:=libtool --tag=CC --mode=compile $(CC)
LT_CC_DEP:=$(CC)
LT_LD:=libtool --tag=CC --mode=link $(CC)
INSTALL:=libtool --tag=CC --mode=install install

MKDIR=mkdir -p

CC=${LT_CC}
CC_DEP=${LT_CC_DEP}
LD=${LT_LD}

CFLAGS+= $(TARGETCFLAGS)
LDFLAGS+= $(TARGETCFLAGS)

LIB_DIR=lib
LIB_DEP_DIR=dep_lib
LIB_OBJ_DIR=obj_lib
SRC_DIR=src
SRC_DEP_DIR=dep_src
SRC_OBJ_DIR=obj_src

FS_LIB=$(wildcard ${LIB_DIR}/*.c)
FS_LIB+=$(FS_LIB_SIMD)
FS_SRC=$(wildcard ${SRC_DIR}/*.c)
FS_OBJ_LIB=$(FS_LIB:${LIB_DIR}/%.c=${LIB_OBJ_DIR}/%.lo)
FS_OBJ_SRC=$(FS_SRC:${SRC_DIR}/%.c=${SRC_OBJ_DIR}/%.lo)
FS_OBJ=$(FS_OBJ_SRC) $(FS_OBJ_LIB)
FS_DEP_LIB=$(FS_LIB:${LIB_DIR}/%.c=${LIB_DEP_DIR}/%.d)
FS_DEP_SRC=$(FS_SRC:${SRC_DIR}/%.c=${SRC_DEP_DIR}/%.d)
FS_DEP=$(FS_DEP_SRC) $(FS_DEP_LIB)

.SUFFIXES: .c .d

.PHONY: all
all: library tools

.PHONY: install
install: all
	$(INSTALL) bin/sha1dcsum $(BINDIR)
	$(INSTALL) bin/sha1dcsum_partialcoll $(BINDIR)
	$(INSTALL) bin/libdetectcoll.la $(LIBDIR)

.PHONY: uninstall
uninstall:
	-$(RM) $(BINDIR)/sha1dcsum
	-$(RM) $(BINDIR)/sha1dcsum_partialcoll
	-$(RM) $(LIBDIR)/libdetectcoll.la

.PHONY: clean
clean:
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
	
.PHONY: check
check: test

.PHONY: tools
tools: sha1dcsum sha1dcsum_partialcoll

.PHONY: sha1dcsum
sha1dcsum: bin/sha1dcsum

.PHONY: sha1dcsum_partialcoll
sha1dcsum_partialcoll: bin/sha1dcsum_partialcoll


.PHONY: library
library: bin/libdetectcoll.la

bin/libdetectcoll.la: $(FS_OBJ_LIB)
	${MKDIR} $(shell dirname $@) && ${LD} ${CFLAGS} $(FS_OBJ_LIB) -o bin/libdetectcoll.la

bin/sha1dcsum: $(FS_OBJ_SRC) library
	${LD} ${CFLAGS} $(FS_OBJ_SRC) $(FS_OBJ_LIB) -Lbin -ldetectcoll -o bin/sha1dcsum

bin/sha1dcsum_partialcoll: bin/sha1dcsum
	-ln -s sha1dcsum bin/sha1dcsum_partialcoll


${SRC_DEP_DIR}/%.d: ${SRC_DIR}/%.c
	${MKDIR} $(shell dirname $@) && $(CC_DEP) $(CFLAGS) -M -MF $@ $<

${SRC_OBJ_DIR}/%.lo ${SRC_OBJ_DIR}/%.o: ${SRC_DIR}/%.c ${SRC_DEP_DIR}/%.d
	${MKDIR} $(shell dirname $@) && $(CC) $(CFLAGS) -o $@ -c $<


${LIB_DEP_DIR}/%.d: ${LIB_DIR}/%.c
	${MKDIR} $(shell dirname $@) && $(CC_DEP) $(CFLAGS) -M -MF $@ $<

${LIB_OBJ_DIR}/%.lo ${LIB_OBJ_DIR}/%.o: ${LIB_DIR}/%.c ${LIB_DEP_DIR}/%.d
	${MKDIR} $(shell dirname $@) && $(CC) $(CFLAGS) -o $@ -c $<

-include $(FS_DEP)
