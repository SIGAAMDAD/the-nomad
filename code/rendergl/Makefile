ifndef win32
OS_INCLUDE=-I/usr/include/ -I/usr/local/include/
COMPILER  =distcc g++
LIB_PREFIX=dependencies/libs/linux
DLL_EXT   =so
else
OS_INCLUDE=-I/usr/x86_64-w64-mingw32/include/
COMPILER  =distcc x86_64-w64-mingw32-g++
LIB_PREFIX=dependencies/libs/windows
DLL_EXT   =dll
endif

ifndef release
DEBUGDEF  =-D_NOMAD_DEBUG
FTYPE     =-Og -g -Wl,--demangle
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
LIB      =rendergl.$(DLL_EXT)

all: $(LIB)

INTERNAL_OBJS=\
	$(O)/rendergl/rgl_main.o \
	$(O)/rendergl/rgl_frame.o \
	$(O)/rendergl/rgl_shader.o \
	$(O)/rendergl/rgl_texture.o \
	$(O)/rendergl/rgl_framebuffer.o \
	$(O)/rendergl/rgl_vertexcache.o \
	$(O)/rendergl/ngl_proc.o \
	$(O)/rendergl/imgui.o \
	$(O)/rendergl/imgui_draw.o \
	$(O)/rendergl/imgui_widgets.o \
	$(O)/rendergl/imgui_tables.o \
	$(O)/rendergl/imgui_impl_sdl2.o \
	$(O)/rendergl/imgui_impl_opengl3.o \
	$(O)/rendergl/stb_image.o \

$(O)/rendergl/%.o: $(SDIR)/rendergl/%.cpp
	$(CC) $(CFLAGS) -shared -fPIC -o $@ -c $<

$(LIB): $(INTERNAL_OBJS)
	$(CC) $(CFLAGS) $(INTERNAL_OBJS)-shared -fPIC -o $(LIB) $(LIB_PREFIX)/libEASTL.a

clean:
	rm -rf $(INTERNAL_OBJS)
	rm $(LIB)
