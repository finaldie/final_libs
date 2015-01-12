MAKE ?= make

INCLUDE_FOLDER = include/flibs
LIB_FOLDER = lib
BENCHMARK_FOLDER = benchmark
API_DOC = doc
TEST_FOLDERS = tests

BENCHMARK_SUFFIX = mail.txt
BENCHMARK = \
    ./benchmark/fhash \
    ./benchmark/flog

SHARED ?= false

# verbose
VERBOSE ?= false
ifeq ($(VERBOSE), false)
	MAKE_FLAGS += -s
endif

include .Makefile.inc
include .Makefile.objs
include .Makefile.libs

#LIB_FOLDERS = \
#  flist \
#  fhash \
#  flock \
#  fmbuf \
#  flog \
#  fconf \
#  ftimer \
#  fthread_pool \
#  fnet \
#  fev \
#  ftu \
#  fcache \
#  fco


.PHONY: clean all check valgrind-check install help benchmark doc doc-clean run_doxygen

all: prepare $(TARGET_LIBS)

prepare:
	@echo "[Compiling $(BUILD_BIT)bit libraries SHARED=$(SHARED)]";
	@echo "CC = $(CC)"
	@echo "MAKE = $(MAKE)"
	@echo "prefix = $(prefix)"
	@echo "VERBOSE = $(VERBOSE)"
	@echo "PLATFORM BIT = $(PLAT_BIT)"
	@echo "DEBUG = $(DEBUG)"
	test -d lib || mkdir -p lib

check:
	@echo "======================Running $(BUILD_BIT)bit Unit Test======================";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDERS) EXT_FLAGS="$(EXT_FLAGS)" $(ASSEMBLY_LOCAL) || exit "$$?";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDERS) $(ASSEMBLY_LOCAL) check;

valgrind-check:
	@echo "======================Running $(BUILD_BIT)bit Valgrind Test======================";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDERS) EXT_FLAGS="$(EXT_FLAGS)" $(ASSEMBLY_LOCAL) || exit "$$?";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDERS) valgrind-check;

clean: benchmark-clean doc-clean clean-flist
	@rm -rf lib
	@echo "clean complete"

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
	    echo "build benchmark for $$bm"; \
	    $(MAKE) $(MAKE_FLAGS) -C $$bm $(ASSEMBLY_LOCAL); \
	done;

benchmark-run:
	@for bm in $(BENCHMARK); do \
	    echo "run benchmark for $$bm"; \
	    $(MAKE) $(MAKE_FLAGS) -C $$bm $(ASSEMBLY_LOCAL) $@; \
	done;

benchmark-clean:
	@for bm in $(BENCHMARK); do \
	    echo "benchmark-clean for $$bm"; \
	    $(MAKE) $(MAKE_FLAGS) -C $$bm clean; \
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

include .Makefile.targets
