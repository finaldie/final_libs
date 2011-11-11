TARGET = libflock.a

INCLUDE_FILES = lmutex.h
INSTALL_PATH = ../final_libraries
INCLUDE_PATH = include
LIBS_PATH = libs

CC = gcc
AR = ar -rcs
INC = 
LL = 
LIBS = 

SRCS = lmutex.c
DEPS = $(patsubst %.c,%.d,$(SRCS))
OBJS = $(patsubst %.c,%.o,$(SRCS))

CFLAGS = -Wall -Werror -g -O2 -std=c99

all:$(TARGET)
$(TARGET):$(OBJS) $(DEPS)
	$(AR) $(TARGET) $(OBJS)

%.d:%.c
	$(CC) -MM $< > $@
%.o:%.c
	$(CC) $(CFLAGS) $(INC) -c $< 

install:
	-(test -d $(INSTALL_PATH) || mkdir $(INSTALL_PATH) )
	-(test -d $(INSTALL_PATH)/$(INCLUDE_PATH) || mkdir $(INSTALL_PATH)/$(INCLUDE_PATH) )
	-(test -d $(INSTALL_PATH)/$(LIBS_PATH) || mkdir $(INSTALL_PATH)/$(LIBS_PATH) )
	cp $(INCLUDE_FILES) $(INSTALL_PATH)/$(INCLUDE_PATH)
	cp $(TARGET) $(INSTALL_PATH)/$(LIBS_PATH)

.PHONY:clean
clean:
	rm -f $(DEPS) $(OBJS) $(TARGET)

help:
	@echo "The information represents in the program:"
	@echo "Final executable name: $(TARGET)"
	@echo "Source files: $(SRCS)"
	@echo "Object files: $(OBJS)"
depend:$(DEPS)
	@echo "Dependencies are now up-to-date"
-include $(DEPS)
