include ../../common/Makefile.common

TARGET = flog.bm

MAIL_FILE_NAME = flog
OUTPUT_FOLDER := $(INSTALL_PATH)/$(BENCHMARK_FOLDER)
OUTPUT_FILE := $(OUTPUT_FOLDER)/$(MAIL_FILE_NAME).$(BENCHMARK_SUFFIX)

INC = -I../../
LL := -L$(INSTALL_PATH)/$(LIBS_PATH) -Wl,-rpath,$(INSTALL_PATH)/$(LIBS_PATH)
FLAGS = -std=c99 -D_BSD_SOURCE
LIBS = \
    -lpthread \
    -lftu \
    -lflist \
    -lflog \
    -lfmbuf \
    -lfhash

SRCS = \
    main.c

benchmark: prepare
	$(CC) -O2 $(COMMON_FLAGS) $(FLAGS) $(INC) $(LL) -o $(TARGET) $(SRCS) $(LDFLAGS) $(LIBS)
	./$(TARGET) 1000000 2 4 | tee $(OUTPUT_FILE)

prepare:
	test -d $(OUTPUT_FOLDER) || mkdir -p $(OUTPUT_FOLDER)

clean:
	rm -f *.o $(TARGET) sync_* async_*

.PHONY: $(TARGET) benchmark clean
