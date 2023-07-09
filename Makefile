COMPILE_ARCH=$(shell uname -m | sed -e 's/i.86/x86/' | sed -e 's/^arm.*/arm/')

VERSION       = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0
CC            = distcc g++
O             = obj
QVM           = qvm
SDIR          = code
SGAME         = sgame.qvm
EXE           = glnomad
LDLIBS        =\
			/usr/local/lib/libSDL2.a \
			/usr/local/lib/libopenal.a \
			/usr/local/lib/libSDL2_ttf.a \
			/usr/local/lib/xallocator.a \
			/usr/local/lib/libfoonathan_memory-0.7.3.a \
			/usr/local/lib/libSOIL.a \
			libEASTL.a \
			base64.a \
			-lbz2 \
			-lz \
			-lGL \
			-logg \
			-lvorbisfile \
			-lfreetype \
			-lvulkan \
			-lsndfile \
			-lpthread \
			-leasy_profiler

LIB_PREFIX      = dependencies/

STATIC_LIBS     =\
			

# engine build options
BUILD_ALLOCATOR = 1
BUILD_RENDERER  = 1

RENDERER = opengl
EXE      = glnomad

ifndef CODE_DIR
SRC_DIR  = code/
endif

.PHONY: all clean targets clean.objs clean.exe

INCLUDE    =-I/usr/include -Idependencies/include -I/usr/local/include -Icode -I.
OPTIMIZERS =-fexpensive-optimizations -funroll-loops -ffast-math -mfma -mavx2
DEFINES    =-D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)
CFLAGS     =-std=c++17 -Og -g -O0

COMPILE_SRC=$(CC) $(CFLAGS) $(INCLUDE) $(DEFINES) $(OPTIMIZERS) -o $@ -c $<
COMPILE_C=gcc -Og -g $(INCLUDE) $(DEFINES) $(OPTIMIZERS) -o $@ -c $<

RENDERGL=\
	$(O)/rendergl/r_framebuffer.o \
	$(O)/rendergl/r_opengl.o \
	$(O)/rendergl/r_vertexcache.o \
	$(O)/rendergl/r_texture.o \
	$(O)/rendergl/r_shader.o \
	$(O)/rendergl/r_spritesheet.o \
	$(O)/rendergl/m_renderer.o \
	$(O)/rendergl/glad.o
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
	$(O)/allocator/z_alloc.o

all: $(EXE)

$(O)/rendergl/%.o: $(SDIR)/src/%.cpp
	$(COMPILE_SRC)
$(O)/common/%.o: $(SDIR)/common/%.cpp
	$(COMPILE_SRC)
$(O)/game/%.o: $(SDIR)/src/%.cpp
	$(COMPILE_SRC)
$(O)/engine/%.o: $(SDIR)/src/%.cpp
	$(COMPILE_SRC)
$(O)/allocator/%.o: $(SDIR)/src/%.cpp
	$(COMPILE_SRC)

$(EXE): $(SRC) $(COMMON) $(RENDERGL)
	$(CC) $(CFLAGS) $(SRC) $(COMMON) $(RENDERGL) -o $(EXE) $(LDLIBS)

clean.exe:
	rm $(EXE)
clean.objs:
	rm -r $(SRC) $(COMMON) $(RENDERGL)
clean:
	rm -r $(SRC) $(COMMON) $(RENDERGL)
	rm $(EXE)