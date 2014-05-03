-include ../common/Makefile.common

TARGET = libftu

INCLUDE_FILES = ftu_inc.h

INC = -I../
LL =
LIBS =

SRCS = ftu_inc.c
OBJS = $(patsubst %.c,%.o,$(SRCS))

FL_FLAGS = $(COMMON_FLAGS) $(CFLAGS) $(EXT_FLAGS)

.PHONY: all install clean help

all: $(TARGET)

$(TARGET): $(OBJS)
ifeq ($(SHARED), true)
	$(CC) $(FL_FLAGS) $(SHARED_FLAGS) -o $(TARGET).so $(OBJS)
else
	$(AR) $(TARGET).a $(OBJS)
endif

%.o: %.c
	$(CC) $(FL_FLAGS) $(INC) -c $<

install:
	-(test -d $(INSTALL_PATH) || mkdir -p $(INSTALL_PATH) )
	-(test -d $(INSTALL_PATH)/$(INCLUDE_PATH) || mkdir -p $(INSTALL_PATH)/$(INCLUDE_PATH) )
	-(test -d $(INSTALL_PATH)/$(LIBS_PATH) || mkdir $(INSTALL_PATH)/$(LIBS_PATH) )
	cp $(INCLUDE_FILES) $(INSTALL_PATH)/$(INCLUDE_PATH)
	cp $(TARGET).* $(INSTALL_PATH)/$(LIBS_PATH)

clean:
	rm -f $(OBJS) $(TARGET).*

help:
	@echo "The information represents in the program:"
	@echo "Final executable name: $(TARGET).a|.so"
	@echo "Source files: $(SRCS)"
	@echo "Object files: $(OBJS)"
