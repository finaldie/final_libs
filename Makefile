MAKE = make
ASSEMBLY_FOLDER = final_libraries

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

all:
	for lib in $(LIB_FOLDERS); do $(MAKE) -C $$lib; $(MAKE) -C $$lib install; done;

run_test:
	$(MAKE) -C test && $(MAKE) -C test run_test

.PHONY:clean
clean:
	for lib in $(LIB_FOLDERS); do $(MAKE) -C $$lib clean; done;
	rm -rf $(ASSEMBLY_FOLDER)
