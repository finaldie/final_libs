MAKE ?= make

INCLUDE_FOLDER = include/flibs
LIB_FOLDER = lib
BENCHMARK_FOLDER = benchmark
API_DOC = doc
TEST_FOLDER = tests

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

all: $(TARGET_LIBS)
	@echo "[Compiling $(BUILD_BIT)bit libraries SHARED=$(SHARED)]";
	@echo "CC = $(CC)"
	@echo "MAKE = $(MAKE)"
	@echo "prefix = $(prefix)"
	@echo "VERBOSE = $(VERBOSE)"
	@echo "PLATFORM BIT = $(PLAT_BIT)"
	@echo "debug = $(debug)"
	test -d $(LIB_FOLDER) || mkdir -p $(LIB_FOLDER)
	cp $(TARGET_LIBS) $(LIB_FOLDER)

check:
	@echo "======================Running $(BUILD_BIT)bit Unit Test======================";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDER) EXT_FLAGS="$(EXT_FLAGS)" $(ASSEMBLY_LOCAL) || exit "$$?";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDER) $(ASSEMBLY_LOCAL) check;

valgrind-check:
	@echo "======================Running $(BUILD_BIT)bit Valgrind Test======================";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDER) EXT_FLAGS="$(EXT_FLAGS)" $(ASSEMBLY_LOCAL) || exit "$$?";
	@$(MAKE) $(MAKE_FLAGS) -C $(TEST_FOLDER) valgrind-check;

clean: clean-flist clean-fcache clean-fhash clean-mbuf clean-fco clean-fnet
clean: clean-ftime clean-flock clean-fthpool clean-fconf
	@rm -rf lib
	@echo "clean complete"

install:
	@test -d $(prefix) || mkdir -p $(prefix)
	@test -d $(prefix)/$(INCLUDE_FOLDER) || mkdir -p $(prefix)/$(INCLUDE_FOLDER)
	@test -d $(prefix)/$(LIB_FOLDER) || mkdir -p $(prefix)/$(LIB_FOLDER)
	@cp -R $(INCLUDE_FOLDER)/* $(prefix)/$(INCLUDE_FOLDER)
	@cp $(LIB_FOLDER)/* $(prefix)/$(LIB_FOLDER)
	@echo "Liraries installed in $(prefix)"

help:
	@echo "make [CC=gcc] [SHARED=true] [debug=true] [BIT=32] [VERBOSE=true]"
	@echo "make check [CC=gcc] [BIT=32] [VERBOSE=true]"
	@echo "make valgrind-check [CC=gcc] [BIT=32] [VERBOSE=true]"
	@echo "make install [BIT=32] [prefix=/usr/local] "
	@echo "make clean"
	@echo "make benchmark"
	@echo "make benchmark-clean"
	@echo "make doc"
	@echo "make doc-clean"

include .Makefile.targets

.PHONY: all prepare check valgrind-check clean install help benchmark doc
