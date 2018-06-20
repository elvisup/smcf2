TOPDIR = .
OUTDIR = $(TOPDIR)/out
CROSS_COMPILE:=mips-linux-gnu-
#CROSS_COMPILE:=

CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip
AR = $(CROSS_COMPILE)ar

CFLAGS := -g -O2 -Wall -lpthread -lrt -ldl

ifeq ($(LIBTYPE), muclibc)
	CFLAGS += -muclibc
endif

INCLUDES := -I$(TOPDIR)/../include/ \
	-I$(TOPDIR)/include

ifeq ($(LIBTYPE), muclibc)
LIBS := -L$(TOPDIR)/../out/lib/uclibc/ -lsmcf
else
LIBS := -L$(TOPDIR)/../out/lib/glibc/ -lsmcf
endif

ifeq ($(LIBTYPE), muclibc)
	TARGET = smcf_demo_uclibc
else
	TARGET = smcf_demo_glibc
endif

OBJS := ./src/module_face_capture/module_face_capture.o \
	./src/module_face_recognition/module_face_recognition.o \
	./src/module_rtsp_video/module_rtsp_video.o \
	./src/module_t01_control/module_t01_control.o \
	./src/module_web_server/module_web_server.o \
	./smcf2_demo.o

all:$(TARGET)

%.o:%.c
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ -c $^

$(TARGET):$(OBJS)
	$(CC) $(OBJS) -o $@ $(INCLUDES) $(LIBS) $(CFLAGS)
	$(STRIP) $(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET)
