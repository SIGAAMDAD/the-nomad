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
			-lGL \
			-logg \
			-lvorbisfile \
			-lfreetype \
			libimgui_dbg.a \
			-lvulkan \
			-lsndfile \

.PHONY: all clean targets clean.objs clean.exe

ifdef release
CFLAGS= -Ofast -s -std=c++17
LDLIBS+=libglad.a
VMFLAGS= -Ofast -s -std=c89
else
CFLAGS= -Og -g -std=c++17 -Wall -Wpedantic -D_NOMAD_DEBUG
LDLIBS+=libglad_dbg.a
VMFLAGS= -Og -g -std=c89 -Wall -Wpedantic -D_NOMAD_DEBUG -DDEBUG_VM
endif
INCLUDE= -I/usr/include -Ideps -Ideps/glad/include -I/usr/local/include -I/usr/include/freetype2 -Isrc -mfma -mavx2
#OPIMTIZERS=-fexpensive-optimizations -funroll-loops -ffast-math -finline-limit=10000
DEFINES    =-D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)
CFLAGS    += $(INCLUDE) $(DEFINES) $(OPIMTIZERS)

SGAME_ASM= \
	$(QVM)/sg_item.q3asm \
	$(QVM)/sg_main.q3asm \
	$(QVM)/sg_mem.q3asm \
	$(QVM)/sg_mthink.q3asm \
	$(QVM)/sg_playr.q3asm \

BFFOBJ= \
	$(O)/read.o \
	$(O)/common.o \

COMMONOBJ= \
	$(O)/vm_run.o \
	$(O)/vm.o \

SRCOBJ= \
	$(O)/g_syscalls.o \
	$(O)/g_sound.o \
	$(O)/g_rng.o \
	$(O)/g_math.o \
	$(O)/g_main.o \
	$(O)/g_init.o \
	$(O)/g_bff.o \
	$(O)/g_game.o \
	\
	$(O)/n_console.o \
	$(O)/n_scf.o \
	$(O)/n_shared.o \
	$(O)/n_common.o \
	\
	$(O)/r_framebuffer.o \
	$(O)/r_opengl.o \
	$(O)/r_vertexcache.o \
	$(O)/r_texture.o \
	$(O)/r_shader.o \
	\
	$(O)/m_renderer.o \
	$(O)/z_heap.o \
	$(O)/z_alloc.o \

all: $(EXE)
targets: $(EXE) $(SGAME)

$(QVM)/%.q3asm: $(SDIR)/sgame/%.c
	./lcc -o $@ $<

$(O)/%.o: $(SDIR)/bff_file/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<
$(O)/%.o: $(SDIR)/src/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<
$(O)/%.o: $(SDIR)/src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
$(O)/vm_run.o: $(SDIR)/common/vm_run.cpp
	$(CC) $(CFLAGS) -o $@ -c $<
$(O)/vm.o: $(SDIR)/common/vm.c
	gcc -std=c89 -ansi -Ofast -s -o $@ -c $<

$(EXE): $(SRCOBJ) $(BFFOBJ) $(COMMONOBJ)
	$(CC) $(CFLAGS) $(SRCOBJ) $(BFFOBJ) $(COMMONOBJ) -o $(EXE) $(LDLIBS)
$(SGAME): $(SGAME_ASM)
	./q3asm -f qvm/compile_sgame.q3asm

clean.exe:
	rm $(EXE)
clean.objs:
	rm -r $(SRCOBJ) $(BFFOBJ) $(COMMONOBJ)
clean:
	rm -r $(SRCOBJ) $(BFFOBJ) $(COMMONOBJ)
	rm $(EXE)