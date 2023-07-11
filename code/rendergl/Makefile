ifeq ($(os),linux)
	OS_INCLUDE=-I/usr/include/ -I/usr/local/include/
	COMPILER  =distcc g++
	LIB_PREFIX=dependencies/linux
	DLL_EXT   =so
else ifeq ($(os),win32)
	OS_INCLUDE=-I/usr/x86_64-w64-mingw32/include/
	COMPILER  =distcc x86_64-w64-mingw32-g++
	LIB_PREFIX=dependencies/windows
	DLL_EXT   =dll
endif

ifeq ($(build),debug)
	DEBUGDEF  =-D_NOMAD_DEBUG
	FTYPE     =-Og -g
else ifeq ($(build),release)
	DEBUGDEF  =
	FTYPE     =-Ofast -s
endif

VERSION       = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0

INCLUDE       =-Idependencies/include/ -Idependencies/include/EA/ $(OS_INCLUDE) -I. -Icode/
VERSION_DEFINE=-D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)

DEFINES       =$(VERSION_DEFINE) $(DEBUGDEF)
OPTIMIZERS    =-fexpensive-optimizations -funroll-loops -ffast-math -mfma -msse3 -msse2 -msse -mavx -mavx2

CC       =$(COMPILER)
CFLAGS   =-std=c++17 $(FTYPE) $(DEFINES) $(INCLUDE) $(OPTIMIZERS)
O        =obj
SDIR     =code
LIB      =rendergl.$(DLL_EXT)

INTERNAL_OBJS=\
	$(O)/rendergl/rgl_main.o \
	$(O)/rendergl/rgl_frame.o \
	$(O)/rendergl/rgl_shader.o \
	$(O)/rendergl/rgl_texture.o \
	$(O)/rendergl/rgl_vertexcache.o \
	$(O)/rendergl/ngl_proc.o \
	$(O)/rendergl/imgui.o \
	$(O)/rendergl/imgui_draw.o \
	$(O)/rendergl/imgui_widgets.o \
	$(O)/rendergl/imgui_tables.o \
	$(O)/rendergl/imgui_impl_sdl2.o \
	$(O)/rendergl/imgui_impl_opengl3.o

ENGINE_OBS=\
	$(O)/engine/n_shared.o

$(O)/rendergl/%.o: $(SDIR)/rendergl/%.cpp
	$(CC) $(CFLAGS) -shared -fPIC -o $@ -c $<
$(O)/engine/%.o: $(SDIR)/src/%.cpp
	$(CC) $(CFLAGS) -shared -fPIC -o $@ -c $<

$(LIB): $(INTERNAL_OBJS) $(ENGINE_OBS)
	$(CC) $(CFLAGS) $(INTERNAL_OBJS) $(ENGINE_OBS) -shared -fPIC -o $(LIB)
