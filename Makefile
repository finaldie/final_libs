MAKE ?= make
SHARED ?= false

INCLUDE_FOLDER = include/flibs
LIB_FOLDER = lib

include .Makefile.inc
include .Makefile.objs
include .Makefile.libs
include .Makefile.tests
include .Makefile.bench

all: $(TARGET_LIBS)
	@echo "[Compiling $(BUILD_BIT)bit libraries SHARED=$(SHARED)]";
	@echo "CC = $(CC)"
	@echo "MAKE = $(MAKE)"
	@echo "prefix = $(prefix)"
	@echo "PLATFORM BIT = $(PLAT_BIT)"
	@echo "debug = $(debug)"
	test -d $(LIB_FOLDER) || mkdir -p $(LIB_FOLDER)
	cp $(TARGET_LIBS) $(LIB_FOLDER)

validate:
	(cd $(LIB_FOLDER) && exit `find . -name "*.so" | xargs ldd -r | grep "undefine" | wc -l`)

check: $(TEST_TARGET)
	@echo "================Running $(BUILD_BIT)bit Unit Test==============";
	@rm -rf tests/logs
	@test -d tests/logs || mkdir tests/logs
	./$(TEST_TARGET)

valgrind-check: $(TEST_TARGET)
	@echo "==============Running $(BUILD_BIT)bit Valgrind Test============";
	@rm -rf tests/logs
	@test -d tests/logs || mkdir tests/logs
	valgrind --tool=memcheck --leak-check=full \
	    --suppressions=./tests/valgrind/fco.suppression \
	    --suppressions=./tests/valgrind/flog.suppression \
	    --suppressions=./tests/valgrind/pthread.suppression \
	    --gen-suppressions=all --error-exitcode=1 ./$(TEST_TARGET)

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

bench: $(BENCH_TARGETS)

bench-run: bench-fhash bench-flog

doc: doc-gen

help:
	@echo "make [CC=gcc] [SHARED=true] [debug=true] [BIT=32]"
	@echo "make check [CC=gcc] [BIT=32]"
	@echo "make valgrind-check [CC=gcc] [BIT=32]"
	@echo "make install [BIT=32] [prefix=/usr/local] "
	@echo "make clean"
	@echo "make bench"
	@echo "make bench-run"
	@echo "make clean-bench"
	@echo "make doc"
	@echo "make clean-doc"

include .Makefile.targets
include .Makefile.doc

.PHONY: all prepare check valgrind-check clean install bench bench-run doc help
