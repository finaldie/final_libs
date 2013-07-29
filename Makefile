MAKE := make

ifeq ($(prefix),)
need_install := false
endif

ASSEMBLY_LOCAL_FOLDER = $(shell pwd)/final_libraries
ASSEMBLY_INCLUDE_FOLDER := include/flibs
ASSEMBLY_32_LIB_FOLDER := lib
ASSEMBLY_64_LIB_FOLDER := lib64
ASSEMBLY32 := INSTALL_PATH=$(ASSEMBLY_LOCAL_FOLDER) INCLUDE_PATH=$(ASSEMBLY_INCLUDE_FOLDER) LIBS_PATH=$(ASSEMBLY_32_LIB_FOLDER)
ASSEMBLY64 := INSTALL_PATH=$(ASSEMBLY_LOCAL_FOLDER) INCLUDE_PATH=$(ASSEMBLY_INCLUDE_FOLDER) LIBS_PATH=$(ASSEMBLY_64_LIB_FOLDER)

ASSEMBLY_FOLDERS := $(prefix)/$(ASSEMBLY_INCLUDE_FOLDER) $(prefix)/$(ASSEMBLY_32_LIB_FOLDER) $(prefix)/$(ASSEMBLY_64_LIB_FOLDER)

PLATFORM = $(shell uname -m)
COMMON32_CFLAGS += -m32
COMMON64_CFLAGS +=

ifeq ($(MODE), debug)
else
	COMMON32_CFLAGS += -O2
	COMMON64_CFLAGS += -O2
endif

# Build all libs by order
LIB_FOLDERS = flist \
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

all: all32 all64

all32:
	echo "========================Compiling 32bit libraries========================"; \
	for lib in $(LIB_FOLDERS); \
	do \
		$(MAKE) -C $$lib $(ASSEMBLY32) clean; \
		$(MAKE) -C $$lib $(ASSEMBLY32) EXT_FLAGS="$(COMMON32_CFLAGS)" || exit "$$?"; \
		$(MAKE) -C $$lib $(ASSEMBLY32) install; \
	done;
ifneq ($(need_install),false)
	test -d $(prefix) || mkdir -p $(ASSEMBLY_FOLDERS)
	cp $(ASSEMBLY_LOCAL_FOLDER)/$(ASSEMBLY_INCLUDE_FOLDER)/* $(prefix)/$(ASSEMBLY_INCLUDE_FOLDER)
	cp $(ASSEMBLY_LOCAL_FOLDER)/$(ASSEMBLY_32_LIB_FOLDER)/* $(prefix)/$(ASSEMBLY_32_LIB_FOLDER)
endif

all64:
ifeq ($(PLATFORM),i386) 
	exit "32 bit platform, abort to compile the 64bit library";
else
	echo "========================Compiling 64bit libraries========================";
	for lib in $(LIB_FOLDERS); \
	do \
		$(MAKE) -C $$lib $(ASSEMBLY64) clean; \
		$(MAKE) -C $$lib $(ASSEMBLY64) EXT_FLAGS="$(COMMON64_CFLAGS)" || exit "$$?"; \
		$(MAKE) -C $$lib $(ASSEMBLY64) install; \
	done;
ifneq ($(need_install),false)
		test -d $(prefix) || mkdir -p $(ASSEMBLY_FOLDERS)
		cp $(ASSEMBLY_LOCAL_FOLDER)/$(ASSEMBLY_INCLUDE_FOLDER)/* $(prefix)/$(ASSEMBLY_INCLUDE_FOLDER)
		cp $(ASSEMBLY_LOCAL_FOLDER)/$(ASSEMBLY_64_LIB_FOLDER)/* $(prefix)/$(ASSEMBLY_64_LIB_FOLDER)
endif
endif


check: all32_check all64_check

all32_check:
	echo "======================Running 32bit Unit Test======================"
	$(MAKE) -C $(TEST_FOLDERS) $(ASSEMBLY32) clean
	$(MAKE) -C $(TEST_FOLDERS) EXT_FLAGS="$(COMMON32_CFLAGS)" $(ASSEMBLY32) || exit "$$?"
	$(MAKE) -C $(TEST_FOLDERS) $(ASSEMBLY32) check

all64_check:
ifeq ($(PLATFORM),i386)
	exit "32 bit platform, abort to running the 64bit Unit Test";
else
	echo "======================Running 64bit Unit Test======================"
	$(MAKE) -C $(TEST_FOLDERS) $(ASSEMBLY64) clean
	$(MAKE) -C $(TEST_FOLDERS) EXT_FLAGS="$(COMMON64_CFLAGS)" $(ASSEMBLY64) || exit "$$?"
	$(MAKE) -C $(TEST_FOLDERS) $(ASSEMBLY64) check
endif

valgrind-check: all32_valgrind_check all64_valgrind_check

all32_valgrind_check:
	echo "======================Running 32bit Valgrind Check======================"
	$(MAKE) -C $(TEST_FOLDERS) $(ASSEMBLY32) clean
	$(MAKE) -C $(TEST_FOLDERS) EXT_FLAGS="$(COMMON32_CFLAGS)" $(ASSEMBLY32) || exit "$$?"
	$(MAKE) -C $(TEST_FOLDERS) $(ASSEMBLY32) PLATFORM=$(PLATFORM) valgrind-check

all64_valgrind_check:
ifeq ($(PLATFORM),i386)
	exit "32 bit platform, abort to running the 64bit Valgrind Check";
else
	echo "======================Running 64bit Valgrind Check======================"
	$(MAKE) -C $(TEST_FOLDERS) $(ASSEMBLY64) clean
	$(MAKE) -C $(TEST_FOLDERS) EXT_FLAGS="$(COMMON64_CFLAGS)" $(ASSEMBLY64) || exit "$$?"
	$(MAKE) -C $(TEST_FOLDERS) $(ASSEMBLY64) PLATFORM=$(PLATFORM) valgrind-check
endif

.PHONY:clean all all32 all64 all32_check all64_check run_test
clean:
	for lib in $(LIB_FOLDERS); do $(MAKE) -C $$lib $(ASSEMBLY64) clean; done;
	$(MAKE) -C $(TEST_FOLDERS) clean
	rm -rf $(ASSEMBLY_LOCAL_FOLDER)
