-include ../common/Makefile.common

TARGET := test

INC := -I$(INSTALL_PATH)/$(INCLUDE_PATH)
LL := -L$(INSTALL_PATH)/$(LIBS_PATH) -Wl,-rpath,$(INSTALL_PATH)/$(LIBS_PATH)

LIBS := -lfhash \
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
	   -lfconf

SRCS := main.c \
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

COPY_FILES :=

OBJS := $(patsubst %.c, %.o, $(SRCS))

CFLAGS += $(EXT_FLAGS)
CFLAGS +=
LDFLAGS +=

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(EXT_FLAGS) $(INC) $(LL) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $<

check:
	./$(TARGET)

valgrind-gensup:
	valgrind --tool=memcheck --leak-check=full --suppressions=./valgrind.suppress --gen-suppressions=yes ./$(TARGET)

valgrind-nosup:
	valgrind --tool=memcheck --leak-check=full ./$(TARGET)

valgrind-check:
	valgrind --tool=memcheck --leak-check=full --suppressions=./valgrind.suppress --error-exitcode=1 -v ./$(TARGET)

.PHONY:clean
clean:
	rm -f $(OBJS) $(TARGET)
	rm -f test_async_log*
	rm -f test_log*

help:
	@echo "The information represents in the program:"
	@echo "Final executable name: $(TARGET)"
	@echo "Source files: $(SRCS)"
	@echo "Object files: $(OBJS)"
