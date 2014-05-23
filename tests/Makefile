include ../common/Makefile.common

TARGET = test

INC := -I$(INSTALL_PATH)/$(INCLUDE_PATH) -I../
LL := -L$(INSTALL_PATH)/$(LIBS_PATH) -Wl,-rpath,$(INSTALL_PATH)/$(LIBS_PATH)

LIBS = \
 -lfmempool \
 -lpthread \
 -lflock \
 -lflog \
 -lfmbuf \
 -lftimer \
 -lrt \
 -lfev \
 -lflist \
 -lfnet \
 -lfcache \
 -lfco \
 -lftu \
 -lfconf \
 -lfhash

SRCS = main.c \
 t_hash.c \
 t_mem.c \
 t_log.c \
 t_mbuf.c \
 t_timer.c \
 t_list.c \
 t_fev.c \
 t_fcache.c \
 t_fco.c \
 t_conf.c

COPY_FILES =

OBJS := $(patsubst %.c, %.o, $(SRCS))

FL_FLAGS = $(COMMON_FLAGS) $(CFLAGS) $(EXT_FLAGS)
LDFLAGS += $(LIBS)

.PHONY: all $(TARGET) check valgrind-gensup valgrind-nosup valgrind-check clean help

all: prepare $(TARGET)

prepare:
	test -d logs || mkdir logs

$(TARGET): $(OBJS)
	$(CC) $(FL_FLAGS) $(INC) $(LL) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(FL_FLAGS) $(INC) -c $<

check:
	./$(TARGET)

valgrind-gensup:
	valgrind --tool=memcheck --leak-check=full --suppressions=./valgrind.suppress --gen-suppressions=yes ./$(TARGET)

valgrind-nosup:
	valgrind --tool=memcheck --leak-check=full ./$(TARGET)

valgrind-check:
	valgrind --tool=memcheck --leak-check=full --suppressions=./valgrind.suppress --error-exitcode=1 ./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf ./logs

help:
	@echo "The information represents in the program:"
	@echo "Final executable name: $(TARGET)"
	@echo "Source files: $(SRCS)"
	@echo "Object files: $(OBJS)"
