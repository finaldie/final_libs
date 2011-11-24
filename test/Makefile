TARGET = test
BIN_PATH = bin

FLIB_PATH = ../final_libraries
INCLUDE_PATH = include
LIBS_PATH = libs
INSTALL_PATH = $(FLIB_PATH)/$(BIN_PATH)

CC = gcc
INC = -I$(FLIB_PATH)/$(INCLUDE_PATH)
LL = -L$(FLIB_PATH)/libs
LIBS = -lfhash \
	   -lfmempool \
	   -lpthread \
	   -lflock \
	   -lflog

SRCS = tu_inc.c \
	   main.c \
	   t_hash.c \
	   t_mem.c \
	   t_log.c

COPY_FILES = 
	   
DEPS = $(patsubst %.c,%.d,$(SRCS))
OBJS = $(patsubst %.c,%.o,$(SRCS))

CFLAGS = -Wall -Werror -g

all:$(TARGET)
$(TARGET):$(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(INC) $(LL) -o $(TARGET) $(OBJS) $(LIBS)

%.d:%.c
	$(CC) $(INC) -MM $< > $@
%.o:%.c 
	$(CC) $(CFLAGS) $(INC) -c $< 

install:
	-(test -d $(INSTALL_PATH) || mkdir -p $(INSTALL_PATH) )
	cp $(TARGET) $(COPY_FILES) $(INSTALL_PATH)

run_test:
	-(cd $(INSTALL_PATH) && ./$(TARGET) )

.PHONY:clean
clean:
	rm -f $(DEPS) $(OBJS) $(TARGET)
	rm -rf $(FLIB_PATH)

help:
	@echo "The information represents in the program:"
	@echo "Final executable name: $(TARGET)"
	@echo "Source files: $(SRCS)"
	@echo "Object files: $(OBJS)"
depend:$(DEPS)
	@echo "Dependencies are now up-to-date"
-include $(DEPS)
