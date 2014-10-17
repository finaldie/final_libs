include common/Makefile.common

MAKE ?= make

INCLUDE_FOLDER = include/flibs
LIB_FOLDER = lib
BENCHMARK_FOLDER = benchmark
API_DOC = doc

ASSEMBLY_LOCAL_FOLDER := $(shell pwd)/final_libraries
ASSEMBLY_LOCAL := INSTALL_PATH=$(ASSEMBLY_LOCAL_FOLDER) INCLUDE_PATH=$(INCLUDE_FOLDER) LIBS_PATH=$(LIB_FOLDER)

prefix ?= $(ASSEMBLY_LOCAL_FOLDER)
ASSEMBLY_FOLDERS := $(prefix)/$(INCLUDE_FOLDER) $(prefix)/$(LIB_FOLDER)

BENCHMARK_SUFFIX = mail.txt
BENCHMARK = \
    ./benchmark/fhash

# PLAT_BIT is one of the 32 or 64
PLAT_BIT = $(shell getconf LONG_BIT)

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

.PHONY: clean all check valgrind-check install help benchmark doc doc-clean run_doxygen

all:
	@echo "[Compiling $(BUILD_BIT)bit libraries SHARED=$(SHARED)]";
	@echo "CC = $(CC)"
	@echo "MAKE = $(MAKE)"
	@echo "prefix = $(prefix)"
	@echo "VERBOSE = $(VERBOSE)"
	@echo "PLATFORM BIT = $(PLAT_BIT)"
	@echo "DEBUG = $(DEBUG)"
	@echo "ASSEMBLY_FOLDERS = $(ASSEMBLY_FOLDERS)"
	@echo "COMMON_FLAGS = $(COMMON_FLAGS)"
	@echo "EXT_FLAGS = $(EXT_FLAGS)"
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

clean: benchmark-clean doc-clean
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
	@test -d $(prefix)/$(INCLUDE_FOLDER) || mkdir -p $(prefix)/$(INCLUDE_FOLDER)
	@test -d $(prefix)/$(LIB_FOLDER) || mkdir -p $(prefix)/$(LIB_FOLDER)
	@cp $(ASSEMBLY_LOCAL_FOLDER)/$(INCLUDE_FOLDER)/* $(prefix)/$(INCLUDE_FOLDER);
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

doc: doc-clean doc-prepare run_doxygen
	cd doc/xml \
	    && ls | grep .xml | grep -vE "dir_|struct|union|index.xml" > doc.list \
	    && for xml_file in `cat doc.list`; \
		do \
		    output_path=$(ASSEMBLY_LOCAL_FOLDER)/$(API_DOC); \
		    output_file_name=`echo "$$xml_file" | sed 's/_8h//g' | sed 's/__/_/g' | sed 's/.xml//g'`; \
		    output_file=$$output_path/$$output_file_name.md; \
		    $(shell pwd)/3rds/convert2markdown/src/xml2markdown.py -f $$xml_file > $$output_file; \
		    echo "generate api doc for $$xml_file, output doc -> $$output_file_name"; \
		done;

doc-prepare:
	test -d $(ASSEMBLY_LOCAL_FOLDER)/$(API_DOC) || mkdir -p $(ASSEMBLY_LOCAL_FOLDER)/$(API_DOC)

run_doxygen:
	doxygen .api_doxyfile

doc-clean:
	@rm -rf doc $(ASSEMBLY_LOCAL_FOLDER)/$(API_DOC)
	@echo "doc clean done"

help:
	@echo "make [CC=gcc] [SHARED=true] [MODE=debug[,release]] [BIT=32] [VERBOSE=true]"
	@echo "make check [CC=gcc] [BIT=32] [VERBOSE=true]"
	@echo "make valgrind-check [CC=gcc] [BIT=32] [VERBOSE=true]"
	@echo "make install [BIT=32] [prefix=/usr/local] "
	@echo "make clean"
	@echo "make benchmark"
	@echo "make benchmark-clean"
	@echo "make doc"
	@echo "make doc-clean"
