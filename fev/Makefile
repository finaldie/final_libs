-include ../common/Makefile.common

TARGET = libfev.a

INCLUDE_FILES = fev.h \
		fev_timer.h \
		fev_buff.h \
		fev_conn.h \
		fev_listener.h

AR = ar -rcs
INC = -I$(INSTALL_PATH)/$(INCLUDE_PATH)
LL = 
LIBS = 

SRCS = fev.c \
	   fev_timer.c \
	   fev_buff.c \
	   fev_conn.c \
	   fev_listener.c
DEPS = $(patsubst %.c,%.d,$(SRCS))
OBJS = $(patsubst %.c,%.o,$(SRCS))

CFLAGS += $(EXT_FLAGS)
CFLAGS += -Wall -Werror -g -O2

all:$(TARGET)
$(TARGET):$(OBJS) $(DEPS)
	$(AR) $(TARGET) $(OBJS)

%.d:%.c
	$(CC) $(INC) -MM $< > $@
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
