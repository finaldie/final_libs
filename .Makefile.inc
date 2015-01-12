debug ?= false
prefix ?= /usr/local

# compiling flags
STD = -std=c99
WARN = -Wall
EXTRA = -pedantic -Wpadded -Werror -Wextra -Wno-unused-parameter
EXTRA += -Wno-unused-function -Wfloat-equal -Winline -Wdisabled-optimization
EXTRA += -Wconversion
OTHER = -pipe -g -ggdb3
CC_SHARED = -fPIC
LD_SHARED = -shared
LDFLAGS +=
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
DEPS_LDFLAGS = -Wl,-rpath,$(prefix)/lib
FL_CFLAGS = $(CFLAGS) $(STD) $(WARN) $(EXTRA) $(MACRO) $(OPT) $(OTHER) $(CC_SHARED) $(INC)
FL_LDFLAGS = $(LDFLAGS) $(OPT) $(SHARE) $(OTHER) $(LD_SHARED) $(DEPS_LDFLAGS)

# CC and LD
FL_CC = $(CC) $(FL_CFLAGS)
FL_LD = $(CC) $(FL_LDFLAGS)

################################################################################
# PLAT_BIT is one of the 32 or 64
PLAT_BIT = $(shell getconf LONG_BIT)
BIT ?= $(PLAT_BIT)

# only in x86_64 platform compile a 32bit app need append -m32 parameter
ifeq ($(PLAT_BIT), 64)

ifeq ($(BIT), 32)
	EXT_FLAGS += -m32
	BUILD_BIT = 32
else
	BUILD_BIT = 64
endif

else
	BUILD_BIT = 32
endif
