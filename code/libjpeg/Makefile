ifndef win32
OS_INCLUDE=-I/usr/include/ -I/usr/local/include/
COMPILER  =distcc gcc
LIB_PREFIX=dependencies/libs/linux
DLL_EXT   =so
O         =bin/obj/unix
else
OS_INCLUDE=-I/usr/x86_64-w64-mingw32/include/
COMPILER  =distcc x86_64-w64-mingw32-gcc
LIB_PREFIX=dependencies/libs/windows
DLL_EXT   =dll
O         =bin/obj/win32
endif

ifndef release
DEBUGDEF  =-D_NOMAD_DEBUG
FTYPE     =-Og -g
else
DEBUGDEF  =
FTYPE     =-Ofast -s
endif

VERSION       = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0

INCLUDE       =-Idependencies/include/ -Idependencies/include/EA/ $(OS_INCLUDE) -I. -Icode/ -Idependencies/include/jdk
VERSION_DEFINE=-D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)

.PHONY: all clean

DEFINES       =$(VERSION_DEFINE) $(DEBUGDEF) -DGDR_DLLCOMPILE -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -D_NOMAD_ENGINE
OPTIMIZERS    =\
				-fexpensive-optimizations \
				-funroll-loops \
				-ffast-math \
				-mfma -msse3 -msse2 -msse -mavx2 -mavx \
				-fno-omit-frame-pointer \
#				-ftree-vectorize \
				-finline-functions \
				-finline-small-functions \

ifeq ($(build),debug)
	OPTIMIZERS +=-fvar-tracking
endif

CC       =$(COMPILER) -std=c99
CFLAGS   =$(FTYPE) $(DEFINES) $(INCLUDE) $(OPTIMIZERS) -fPIC
SDIR     =code
LIB      =libjpeg.$(DLL_EXT)

all: $(LIB)

INTERNAL_OBJS=\
	$(O)/libjpeg/jaricom.o \
  	$(O)/libjpeg/jcapimin.o \
  	$(O)/libjpeg/jcapistd.o \
  	$(O)/libjpeg/jcarith.o \
  	$(O)/libjpeg/jccoefct.o  \
  	$(O)/libjpeg/jccolor.o \
  	$(O)/libjpeg/jcdctmgr.o \
  	$(O)/libjpeg/jchuff.o   \
  	$(O)/libjpeg/jcinit.o \
  	$(O)/libjpeg/jcmainct.o \
  	$(O)/libjpeg/jcmarker.o \
  	$(O)/libjpeg/jcmaster.o \
  	$(O)/libjpeg/jcomapi.o \
  	$(O)/libjpeg/jcparam.o \
  	$(O)/libjpeg/jcprepct.o \
  	$(O)/libjpeg/jcsample.o \
  	$(O)/libjpeg/jctrans.o \
  	$(O)/libjpeg/jdapimin.o \
  	$(O)/libjpeg/jdapistd.o \
  	$(O)/libjpeg/jdarith.o \
  	$(O)/libjpeg/jdatadst.o \
  	$(O)/libjpeg/jdatasrc.o \
  	$(O)/libjpeg/jdcoefct.o \
  	$(O)/libjpeg/jdcolor.o \
  	$(O)/libjpeg/jddctmgr.o \
  	$(O)/libjpeg/jdhuff.o \
  	$(O)/libjpeg/jdinput.o \
  	$(O)/libjpeg/jdmainct.o \
  	$(O)/libjpeg/jdmarker.o \
  	$(O)/libjpeg/jdmaster.o \
  	$(O)/libjpeg/jdmerge.o \
  	$(O)/libjpeg/jdpostct.o \
  	$(O)/libjpeg/jdsample.o \
  	$(O)/libjpeg/jdtrans.o \
  	$(O)/libjpeg/jerror.o \
  	$(O)/libjpeg/jfdctflt.o \
  	$(O)/libjpeg/jfdctfst.o \
  	$(O)/libjpeg/jfdctint.o \
  	$(O)/libjpeg/jidctflt.o \
  	$(O)/libjpeg/jidctfst.o \
  	$(O)/libjpeg/jidctint.o \
  	$(O)/libjpeg/jmemmgr.o \
  	$(O)/libjpeg/jmemnobs.o \
  	$(O)/libjpeg/jquant1.o \
  	$(O)/libjpeg/jquant2.o \
  	$(O)/libjpeg/jutils.o

$(O)/libjpeg/%.o: $(SDIR)/libjpeg/%.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ -c $<

$(LIB): $(INTERNAL_OBJS)
	$(CC) $(CFLAGS) $(INTERNAL_OBJS) -shared -fPIC -o $(LIB)

clean:
	rm -rf $(INTERNAL_OBJS)
	rm $(LIB)
