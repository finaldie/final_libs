MAKE = make

INCLUDE_FOLDER = include/flibs
LIB32_FOLDER = lib
LIB64_FOLDER = lib64
BENCHMARK_FOLDER = benchmark

ASSEMBLY_LOCAL_FOLDER := $(shell pwd)/final_libraries
ASSEMBLY_LOCAL32 := INSTALL_PATH=$(ASSEMBLY_LOCAL_FOLDER) INCLUDE_PATH=$(INCLUDE_FOLDER) LIBS_PATH=$(LIB32_FOLDER)
ASSEMBLY_LOCAL64 := INSTALL_PATH=$(ASSEMBLY_LOCAL_FOLDER) INCLUDE_PATH=$(INCLUDE_FOLDER) LIBS_PATH=$(LIB64_FOLDER)

prefix ?= $(ASSEMBLY_LOCAL_FOLDER)
ASSEMBLY_FOLDERS := $(prefix)/$(INCLUDE_FOLDER) $(prefix)/$(LIB32_FOLDER) $(prefix)/$(LIB64_FOLDER)

BENCHMARK_SUFFIX = mail.txt
BENCHMARK = \
    ./benchmark/fhash

# PLAT_BIT is one of the 32 or 64
PLAT_BIT = $(shell getconf LONG_BIT)

# only in x86_64 platform compile a 32bit app need append -m32 parameter
ifeq ($(PLAT_BIT), 64)

ifeq ($(BIT), 32)
	EXT_FLAGS += -m32
	ASSEMBLY_LOCAL = $(ASSEMBLY_LOCAL32)
	LIB_FOLDER := $(LIB32_FOLDER)
	BUILD_BIT = 32
else
	ASSEMBLY_LOCAL = $(ASSEMBLY_LOCAL64)
	LIB_FOLDER := $(LIB64_FOLDER)
	BUILD_BIT = 64
endif

else
	ASSEMBLY_LOCAL = $(ASSEMBLY_LOCAL32)
	LIB_FOLDER := $(LIB32_FOLDER)
	BUILD_BIT = 32
endif

# debug or not
DEBUG ?= false
ifeq ($(MODE), debug)
	EXT_FLAGS += -DNDEBUG -Wstack-protector
else
	EXT_FLAGS += -O2 -Werror
endif

# expose the CC
CC ?= gcc
export CC

# expose the SHARED to everywhere
SHARED ?= false
ifeq ($(SHARED), true)
	EXT_FLAGS += -fPIC
	SHARED_FLAGS = -shared
endif
export SHARED
export SHARED_FLAGS

# verbose
VERBOSE ?= false
ifeq ($(VERBOSE), false)
	MAKE_FLAGS += -s
endif
export VERBOSE

# Build all libs by order
LIB_FOLDERS = \
  flist \
  fhash \
  flock \
  fmbuf \
  fmbuf \
  flog \
  fconf \
  ftimer \
  fmempool \
  fthread_pool \
  fnet \
  fev \
  ftu \
  fcache \
  fco \
  fpcap_conv

TEST_FOLDERS = tests

.PHONY: clean all check valgrind-check install help benchmark

all:
	@echo "[Compiling $(BUILD_BIT)bit libraries SHARED=$(SHARED)]";
	@for lib in $(LIB_FOLDERS); \
	do \
		echo "$(CC) $$lib"; \
		$(MAKE) $(MAKE_FLAGS) -C $$lib $(ASSEMBLY_LOCAL) EXT_FLAGS="$(EXT_FLAGS)" || exit "$$?"; \
		$(MAKE) $(MAKE_FLAGS) -C $$lib $(ASSEMBLY_LOCAL) install; \
	done;

check:
	@echo "======================Running $(BUILD_BIT)bit Unit Test======================";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDERS) EXT_FLAGS="$(EXT_FLAGS)" $(ASSEMBLY_LOCAL) || exit "$$?";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDERS) $(ASSEMBLY_LOCAL) check;

valgrind-check:
	@echo "======================Running $(BUILD_BIT)bit Valgrind Test======================";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDERS) EXT_FLAGS="$(EXT_FLAGS)" $(ASSEMBLY_LOCAL) || exit "$$?";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDERS) valgrind-check;

clean:
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDERS) $(ASSEMBLY_LOCAL) clean;
	@for lib in $(LIB_FOLDERS); \
	do \
		echo "clean $$lib"; \
		$(MAKE) $(MAKE_FLAGS) -C $$lib $(ASSEMBLY_LOCAL) clean; \
	done;
	@rm -rf $(ASSEMBLY_LOCAL_FOLDER);
	@echo "clean complete";

install:
	@test -d $(prefix) || mkdir -p $(ASSEMBLY_FOLDERS);
	@cp $(ASSEMBLY_LOCAL_FOLDER)/$(ASSEMBLY_INCLUDE_FOLDER)/* $(prefix)/$(ASSEMBLY_INCLUDE_FOLDER);
	@cp $(ASSEMBLY_LOCAL_FOLDER)/$(LIB_FOLDER)/* $(prefix)/$(LIB_FOLDER);
	@echo "Liraries installed in $(prefix)";

benchmark: ASSEMBLY_LOCAL += BENCHMARK_SUFFIX=$(BENCHMARK_SUFFIX)
benchmark: ASSEMBLY_LOCAL += BENCHMARK_FOLDER=$(BENCHMARK_FOLDER)
benchmark:
	@for bm in $(BENCHMARK); do \
	    echo "benchmark for $$bm"; \
	    $(MAKE) $(MAKE_FLAGS) -C $(BENCHMARK) $(ASSEMBLY_LOCAL); \
	done;

benchmark-clean:
	@for bm in $(BENCHMARK); do \
	    echo "benchmark-clean for $$bm"; \
	    $(MAKE) $(MAKE_FLAGS) -C $(BENCHMARK) clean; \
	done;
	@rm -rf $(ASSEMBLY_LOCAL_FOLDER)/$(BENCHMARK_FOLDER)

help:
	@echo "make [CC=gcc] [SHARED=true] [MODE=debug[,release]] [BIT=32] [VERBOSE=true]"
	@echo "make check [CC=gcc] [BIT=32] [VERBOSE=true]"
	@echo "make valgrind-check [CC=gcc] [BIT=32] [VERBOSE=true]"
	@echo "make install [BIT=32] [prefix=/usr/local] "
	@echo "make clean"
	@echo "make benchmark"
	@echo "make benchmark-clean"
