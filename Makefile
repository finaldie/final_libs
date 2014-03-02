MAKE := make

ifeq ($(prefix),)
need_install := false
endif

ASSEMBLY_LOCAL_FOLDER := $(shell pwd)/final_libraries
ASSEMBLY_INCLUDE_FOLDER := include/flibs
ASSEMBLY_32_LIB_FOLDER := lib
ASSEMBLY_64_LIB_FOLDER := lib64
ASSEMBLY32 := INSTALL_PATH=$(ASSEMBLY_LOCAL_FOLDER) INCLUDE_PATH=$(ASSEMBLY_INCLUDE_FOLDER) LIBS_PATH=$(ASSEMBLY_32_LIB_FOLDER)
ASSEMBLY64 := INSTALL_PATH=$(ASSEMBLY_LOCAL_FOLDER) INCLUDE_PATH=$(ASSEMBLY_INCLUDE_FOLDER) LIBS_PATH=$(ASSEMBLY_64_LIB_FOLDER)
ASSEMBLY_FOLDERS := $(prefix)/$(ASSEMBLY_INCLUDE_FOLDER) $(prefix)/$(ASSEMBLY_32_LIB_FOLDER) $(prefix)/$(ASSEMBLY_64_LIB_FOLDER)

# PLATFORM is one of the 32 or 64
PLATFORM = $(shell getconf LONG_BIT)

# only in x86_64 platform compile a 32bit app need append -m32 parameter
ifeq ($PLATFORM, 64)
	COMMON32_CFLAGS += -m32;
endif
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
	@echo "[Compiling 32bit libraries]";
	@for lib in $(LIB_FOLDERS); \
	do \
		echo "CC $$lib"; \
		$(MAKE) -s -C $$lib $(ASSEMBLY32) clean; \
		$(MAKE) -s -C $$lib $(ASSEMBLY32) EXT_FLAGS="$(COMMON32_CFLAGS)" || exit "$$?"; \
		$(MAKE) -s -C $$lib $(ASSEMBLY32) install; \
	done;
ifneq ($(need_install),false)
	@test -d $(prefix) || mkdir -p $(ASSEMBLY_FOLDERS);
	@cp $(ASSEMBLY_LOCAL_FOLDER)/$(ASSEMBLY_INCLUDE_FOLDER)/* $(prefix)/$(ASSEMBLY_INCLUDE_FOLDER);
	@cp $(ASSEMBLY_LOCAL_FOLDER)/$(ASSEMBLY_32_LIB_FOLDER)/* $(prefix)/$(ASSEMBLY_32_LIB_FOLDER);
	@echo "Liraries installed in $(prefix)";
endif

all64:
ifeq ($(PLATFORM),32) 
	@echo "32 bit platform, abort to compile the 64bit library";
	@exit 0;
else
	@echo "[Compiling 64bit libraries]";
	@for lib in $(LIB_FOLDERS); \
	do \
		echo "CC $$lib"; \
		$(MAKE) -s -C $$lib $(ASSEMBLY64) clean; \
		$(MAKE) -s -C $$lib $(ASSEMBLY64) EXT_FLAGS="$(COMMON64_CFLAGS)" || exit "$$?"; \
		$(MAKE) -s -C $$lib $(ASSEMBLY64) install; \
	done;
ifneq ($(need_install),false)
	@test -d $(prefix) || mkdir -p $(ASSEMBLY_FOLDERS);
	@cp $(ASSEMBLY_LOCAL_FOLDER)/$(ASSEMBLY_INCLUDE_FOLDER)/* $(prefix)/$(ASSEMBLY_INCLUDE_FOLDER);
	@cp $(ASSEMBLY_LOCAL_FOLDER)/$(ASSEMBLY_64_LIB_FOLDER)/* $(prefix)/$(ASSEMBLY_64_LIB_FOLDER);
	@echo "Liraries installed in $(prefix)";
endif
endif


check: check32 check64

check32:
	@echo "======================Running 32bit Unit Test======================";
	@$(MAKE) -s -C $(TEST_FOLDERS) $(ASSEMBLY32) clean;
	@$(MAKE) -s -C $(TEST_FOLDERS) EXT_FLAGS="$(COMMON32_CFLAGS)" $(ASSEMBLY32) || exit "$$?";
	@$(MAKE) -s -C $(TEST_FOLDERS) $(ASSEMBLY32) check;

check64:
ifeq ($(PLATFORM),32)
	@echo "32 bit platform, abort to running the 64bit Unit Test";
	@exit 0
else
	@echo "======================Running 64bit Unit Test======================";
	@$(MAKE) -s -C $(TEST_FOLDERS) $(ASSEMBLY64) clean;
	@$(MAKE) -s -C $(TEST_FOLDERS) EXT_FLAGS="$(COMMON64_CFLAGS)" $(ASSEMBLY64) || exit "$$?";
	@$(MAKE) -s -C $(TEST_FOLDERS) $(ASSEMBLY64) check;
endif

valgrind-check: valgrind-check32 valgrind-check64

valgrind-check32:
	@echo "======================Running 32bit Valgrind Test======================";
	@$(MAKE) -s -C $(TEST_FOLDERS) $(ASSEMBLY32) clean;
	@$(MAKE) -s -C $(TEST_FOLDERS) EXT_FLAGS="$(COMMON32_CFLAGS)" $(ASSEMBLY32) || exit "$$?";
	@$(MAKE) -s -C $(TEST_FOLDERS) valgrind-check;

valgrind-check64:
	@echo "======================Running 64bit Valgrind Test======================";
	@$(MAKE) -s -C $(TEST_FOLDERS) $(ASSEMBLY64) clean;
	@$(MAKE) -s -C $(TEST_FOLDERS) EXT_FLAGS="$(COMMON64_CFLAGS)" $(ASSEMBLY64) || exit "$$?";
	@$(MAKE) -s -C $(TEST_FOLDERS) valgrind-check;

clean:
	@$(MAKE) -s -C $(TEST_FOLDERS) $(ASSEMBLY64) clean;
	@for lib in $(LIB_FOLDERS); \
	do \
		echo "clean $$lib"; \
		$(MAKE) -s -C $$lib $(ASSEMBLY64) clean; \
	done;
	@rm -rf $(ASSEMBLY_LOCAL_FOLDER);
	@echo "clean complete";

install32:
	@for lib in $(LIB_FOLDERS); \
	do \
		echo "Install $$lib"; \
		$(MAKE) -s -C $$lib $(ASSEMBLY32) install; \
	done;

install64:
	@for lib in $(LIB_FOLDERS); \
	do \
		echo "Install $$lib"; \
		$(MAKE) -s -C $$lib $(ASSEMBLY64) install; \
	done;

.PHONY:clean all all32 all64 check check32 check64 valgrind-check valgrind-check32 valgrind-check64 install32 install64
