-include ../common/Makefile.common

TARGET = libfnet

INCLUDE_FILES = fnet_core.h
INSTALL_PATH = ../final_libraries
INCLUDE_PATH = include
LIBS_PATH = libs

INC = -I$(INSTALL_PATH)/$(INCLUDE_PATH)
LL = 
LIBS = 

SRCS = fnet_core.c
OBJS = $(patsubst %.c,%.o,$(SRCS))

CFLAGS += $(EXT_FLAGS)
CFLAGS +=

all: $(TARGET)

$(TARGET): $(OBJS)
ifeq ($(SHARED), true)
	$(CC) $(CFLAGS) -shared -fPIC -o $(TARGET).so $(OBJS)
else
	$(AR) $(TARGET).a $(OBJS)
endif

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< 

install:
	-(test -d $(INSTALL_PATH) || mkdir -p $(INSTALL_PATH) )
	-(test -d $(INSTALL_PATH)/$(INCLUDE_PATH) || mkdir -p $(INSTALL_PATH)/$(INCLUDE_PATH) )
	-(test -d $(INSTALL_PATH)/$(LIBS_PATH) || mkdir $(INSTALL_PATH)/$(LIBS_PATH) )
	cp $(INCLUDE_FILES) $(INSTALL_PATH)/$(INCLUDE_PATH)
	cp $(TARGET).* $(INSTALL_PATH)/$(LIBS_PATH)

.PHONY: all install clean help
clean:
	rm -f $(OBJS) $(TARGET).*

help:
	@echo "The information represents in the program:"
	@echo "Final executable name: $(TARGET).a|.so"
	@echo "Source files: $(SRCS)"
	@echo "Object files: $(OBJS)"
