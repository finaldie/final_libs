debug ?= false
prefix ?= /usr/local
topdir = $(shell pwd)

# compiling flags
STD = -std=c99
WARN = -Wall
EXTRA = -pedantic -Wpadded -Werror -Wextra -Wno-unused-parameter
EXTRA += -Wno-unused-function -Wfloat-equal -Winline -Wdisabled-optimization
EXTRA += -Wconversion
OTHER = -pipe -g -ggdb3
CC_SHARED = -fPIC
MACRO = -D_POSIX_C_SOURCE=200809L -D_BSD_SOURCE
INC = -I./include

ifeq ($(debug), false)
OPT = -O2
EXTRA += -fstack-protector
else
EXTRA += -fstack-protector-all
MACRO += -O0 -finstrument-functions
endif

# linking flags
LD_SHARED = -shared

DEP_LDFLAGS = \
    -Wl,-rpath,$(prefix)/lib \
    -Wl,-rpath,$(topdir)/lib \
    -Lflist \
    -Lfhash \
    -Lfcache \
    -Lfmbuf \
    -Lflock \
    -Lftime \
    -Lfnet \
    -Lfconf \
    -Lfthread_pool \
    -Lflog \
    -Lfev \
    -Lftu

FL_CFLAGS = $(CFLAGS) $(STD) $(WARN) $(EXTRA) $(MACRO) $(OPT) $(OTHER) $(CC_SHARED) $(INC)
FL_LDFLAGS = $(LDFLAGS) $(OPT) $(OTHER) $(LD_SHARED) $(DEP_LDFLAGS)
FL_BIN_LDFLAGS = $(LDFLAGS) $(OPT) $(OTHER) $(DEP_LDFLAGS)

# CC and LD
FL_CC = $(CC) $(FL_CFLAGS)
FL_LD = $(CC) $(FL_LDFLAGS)
FL_BIN_LD = $(CC) $(FL_BIN_LDFLAGS)

################################################################################
# PLAT_BIT is one of the 32 or 64
PLAT_BIT = $(shell getconf LONG_BIT)
BIT ?= $(PLAT_BIT)

# only in x86_64 platform compile a 32bit app need append -m32 parameter
ifeq ($(PLAT_BIT), 64)

ifeq ($(BIT), 32)
	OTHER += -m32
	BUILD_BIT = 32
else
	BUILD_BIT = 64
endif

else
	BUILD_BIT = 32
endif
