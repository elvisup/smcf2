TOPDIR = .
CROSS_COMPILE:=mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

CFLAGS := -g -O2 -Wall -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
ARFLAGS := -rcs

ifeq ($(LIBTYPE), muclibc)
	CFLAGS += -muclibc
endif

INCLUDES := -I$(TOPDIR)/src/inc/ \
	    -I$(TOPDIR)/include

OBJS := $(TOPDIR)/src/core.o \
	$(TOPDIR)/src/async_msg.o \
	$(TOPDIR)/src/sync_msg.o \
	$(TOPDIR)/src/data.o \
	$(TOPDIR)/src/mem.o \
	$(TOPDIR)/src/module_manager.o \
	$(TOPDIR)/src/utils/node_list/node_list.o

ifeq ($(LIBTYPE), muclibc)
	TARGET := ./out/lib/uclibc/libsmcf.a
else
	TARGET := ./out/lib/glibc/libsmcf.a
endif

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $^

all:$(TARGET)
	cp -rf include/ out/

$(TARGET):$(OBJS)
	$(shell if [ ! -d "out/lib/glibc" ]; then mkdir -p out/lib/glibc; fi;)
	$(shell if [ ! -d "out/lib/uclibc" ]; then mkdir -p out/lib/uclibc; fi;)
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -rf $(OBJS) $(TARGET) out/
	find $(TOPDIR) -name "*.[o,a]" | xargs rm -vf
