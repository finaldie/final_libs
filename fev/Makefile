-include ../common/Makefile.common

TARGET = libfev

INCLUDE_FILES = fev.h \
		fev_timer.h \
		fev_buff.h \
		fev_conn.h \
		fev_listener.h \
		fev_timer_service.h \
		fev_tmsvc_types.h

INC = -I../
LL = 
LIBS = 

SRCS = fev.c \
	fev_timer.c \
	fev_buff.c \
	fev_conn.c \
	fev_listener.c \
	fev_tmsvc_modules.c \
	fev_timer_service.c \
	fev_tmsvc_sl_mod.c

OBJS = $(patsubst %.c,%.o,$(SRCS))

FL_FLAGS = $(COMMON_FLAGS) $(CFLAGS) $(EXT_FLAGS)

.PHONY: all install clean help

all: $(TARGET)

$(TARGET): $(OBJS)
ifeq ($(SHARED), true)
	$(CC) $(FL_FLAGS) -o $(TARGET).so $(OBJS)
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
