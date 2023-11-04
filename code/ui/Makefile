ifndef win32
OS_INCLUDE=-I/usr/include/ -I/usr/local/include/
COMPILER  =distcc g++
LIB_PREFIX=dependencies/libs/linux
DLL_EXT   =so
else
OS_INCLUDE=-I/usr/x86_64-w64-mingw32/include/
COMPILER  =distcc x86_64-w64-mingw32-gcc
LIB_PREFIX=dependencies/libs/windows
DLL_EXT   =dll
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

DEFINES       =$(VERSION_DEFINE) $(DEBUGDEF) -DGDR_DLLCOMPILE
OPTIMIZERS    =\
				-fexpensive-optimizations \
				-funroll-loops \
				-ffast-math \
				-mfma -msse3 -msse2 -msse -mavx -mavx2
#				-ftree-vectorize \
				-finline-functions \
				-finline-small-functions \

ifeq ($(build),debug)
	OPTIMIZERS +=-fvar-tracking
endif

CC       =$(COMPILER) -std=c++17
CFLAGS   =$(FTYPE) $(DEFINES) $(INCLUDE) $(OPTIMIZERS)
O        =obj
SDIR     =code
LIB      =glnomad_ui.$(DLL_EXT)

all: $(LIB)

INTERNAL_OBJS=\
	$(O)/ui/ui_lib.o \
	$(O)/ui/ui_main.o \
	$(O)/ui/ui_menu.o \
	$(O)/ui/n_shared.o \
	$(O)/ui/n_math.o \

$(O)/ui/%.o: $(SDIR)/engine/%.c
	distcc gcc $(CFLAGS) -shared -fPIC -o $@ -c $<
$(O)/ui/%.o: $(SDIR)/ui/%.cpp
	$(CC) $(CFLAGS) -shared -fPIC -o $@ -c $<

$(LIB): $(INTERNAL_OBJS)
	$(CC) $(CFLAGS) $(INTERNAL_OBJS) -shared -fPIC -o $(LIB)

clean:
	rm -rf $(INTERNAL_OBJS)
	rm $(LIB)
