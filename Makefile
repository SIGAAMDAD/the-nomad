
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

CFLAGS        =-std=c++17 $(FTYPE) $(DEFINES) $(INCLUDE) $(OPTIMIZERS)
CC            =$(COMPILER)
O             = obj
QVM           = qvm
SDIR          = code
EXE           = glnomad
COMPILE_SRC   =$(CC) $(CFLAGS) -o $@ -c $<
COMPILE_LIBSRC=$(CC) -fPIC -shared $(CFLAGS) -o $@ -c $<

USE_OPENGL      = 1
USE_VULKAN      = 0

RENDERGL_OBJS=\
	$(O)/rendergl/rgl_main.o

RENDER_OBJS=$(RENDERGL_OBJS)
RENDER_LIB =rendergl.$(DLL_EXT)
RENDER_DIR =$(SDIR)/rendergl
RENDER_OUT =$(O)/rendergl


ifeq ($(os),linux)
	LDLIBS=-L/usr/lib/x86_64-linux-gnu/ \
			-lz \
			-lbz2 \
			-lGL \
			-lboost_thread \
			$(LIB_PREFIX)/libEASTL.a \
			$(LIB_PREFIX)/libjemalloc.a \
			$(LIB_PREFIX)/libzstd.a \
			$(LIB_PREFIX)/libopenal.a \
			-L$(LIB_PREFIX) \
			-limgui \
			-lsteam_api64 \
			-lxallocator \
			-lSDL2 \
			-lSDL2_image \
			-lSDL2_ttf \
			-lsndfile \
			-lglad
else ifeq ($(os),win32)
	LDLIBS=-L$(LIB_PREFIX) \
			-lSDL2 \
			-limgui \
			-lz \
			-lbz2 \
			-lSDL2_image \
			-lSDL2_ttf \
			-lopenal \
			-lGL \
			-lsndfile \
			-lxallocator \
			-lzstd \
			-lsteam_api64 \
			-lboost_thread \
			-lglad
endif

.PHONY: all clean targets clean.objs clean.exe clean.pch pch

COMPILE_SRC=$(CC) $(CFLAGS) -o $@ -c $<

COMMON=\
	$(O)/common/vm_run.o \
	$(O)/common/vm.o
SRC=\
	$(O)/game/g_syscalls.o \
	$(O)/game/g_sound.o \
	$(O)/game/g_rng.o \
	$(O)/game/g_math.o \
	$(O)/game/g_main.o \
	$(O)/game/g_init.o \
	$(O)/game/g_bff.o \
	$(O)/game/g_game.o \
	\
	$(O)/engine/n_console.o \
	$(O)/engine/n_scf.o \
	$(O)/engine/n_shared.o \
	$(O)/engine/n_common.o \
	$(O)/engine/n_files.o \
	$(O)/engine/n_map.o \
	\
	$(O)/allocator/z_heap.o \
	$(O)/allocator/z_alloc.o \

all: $(EXE) $(RENDERGL_LIB)

$(O)/rendergl/%.o: $(SDIR)/rendergl/%.cpp
	$(COMPILE_LIB)
$(O)/common/%.o: $(SDIR)/common/%.cpp
	$(COMPILE_SRC)
$(O)/game/%.o: $(SDIR)/src/%.cpp
	$(COMPILE_SRC)
$(O)/engine/%.o: $(SDIR)/src/%.cpp
	$(COMPILE_SRC)
$(O)/allocator/%.o: $(SDIR)/src/%.cpp
	$(COMPILE_SRC)

$(RENDERGL_LIB): $(RENDERGL_OBJS)
	$(CC) $(CFLAGS) -fPIC -shared $(RENDERGL_OBJS) -o $(RENDERGL_LIB)

$(EXE): $(SRC) $(COMMON) $(RENDERGL_LIB)
	$(CC) $(CFLAGS) $(SRC) $(COMMON) -o $(EXE) $(LDLIBS) -L. $(RENDERGL_LIB)

clean.pch:
	rm $(SDIR)/src/n_pch_all.h.gch
clean.exe:
	rm $(EXE)
clean.objs:
	rm -r $(SRC) $(COMMON) $(RENDERGL)
clean:
	rm -r $(SRC) $(COMMON) $(RENDERGL)
	rm $(EXE)