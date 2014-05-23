include ../../common/Makefile.common

TARGET = fhash.bm

MAIL_FILE_NAME = fhash
OUTPUT_FOLDER := $(INSTALL_PATH)/$(BENCHMARK_FOLDER)
OUTPUT_FILE := $(OUTPUT_FOLDER)/$(MAIL_FILE_NAME).$(BENCHMARK_SUFFIX)

KEY_VALUE_SOURCE = /tmp/fhash_kv.txt

INC = -I../../
LL := -L$(INSTALL_PATH)/$(LIBS_PATH) -Wl,-rpath,$(INSTALL_PATH)/$(LIBS_PATH)

SRCS = \
    main.c

LDFLAGS = \
    -lrt \
    -lpthread \
    -lftimer \
    -lfhash \

.PHONY: benchmark prepare clean $(TARGET)

benchmark: prepare
	$(CC) -O2 $(COMMON_FLAGS) $(INC) $(LL) -o $(TARGET) $(SRCS) $(LDFLAGS)
	./$(TARGET) | tee $(OUTPUT_FILE)

prepare:
	@echo "generate random key/values..."
	test -f $(KEY_VALUE_SOURCE) || ./random_string_gen.sh > $(KEY_VALUE_SOURCE)
	@echo "generate random key/values done"
	test -d $(OUTPUT_FOLDER) || mkdir -p $(OUTPUT_FOLDER)

clean:
	rm -f *.o $(TARGET)
