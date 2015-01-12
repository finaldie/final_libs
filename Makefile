MAKE ?= make

INCLUDE_FOLDER = include/flibs
LIB_FOLDER = lib

BENCHMARK_FOLDER = benchmark
API_DOC = doc

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
include .Makefile.tests

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

check: $(TEST_TARGET)
	@echo "================Running $(BUILD_BIT)bit Unit Test==============";
	@rm -rf tests/logs
	@test -d tests/logs || mkdir tests/logs
	./$(TEST_TARGET)

valgrind-check: $(TEST_TARGET)
	@echo "==============Running $(BUILD_BIT)bit Valgrind Test============";
	@rm -rf tests/logs
	@test -d tests/logs || mkdir tests/logs
	valgrind --tool=memcheck --leak-check=full --suppressions=./tests/valgrind.suppress --gen-suppressions=all --error-exitcode=1 ./$(TEST_TARGET)

clean: clean-flist clean-fcache clean-fhash clean-fmbuf clean-fco clean-fnet
clean: clean-ftime clean-flock clean-fthpool clean-fconf clean-flog clean-fev
clean: clean-fut clean-tests
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
	@echo "make benchmark-run"
	@echo "make benchmark-clean"
	@echo "make doc"
	@echo "make doc-clean"

include .Makefile.targets

.PHONY: all prepare check valgrind-check clean install help benchmark doc
