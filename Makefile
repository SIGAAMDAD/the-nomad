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
			-lvulkan \
			-lsndfile \
			-lbz2 \
			-lSDL3 \
			-lpthread \
			-leasy_profiler \


.PHONY: all clean targets clean.objs clean.exe

INCLUDE    =-I/usr/include -Ideps -I/usr/local/include -I/usr/include/freetype2 -Isrc
OPTIMIZERS =-fexpensive-optimizations -funroll-loops -ffast-math -mfma -mavx2
DEFINES    =-D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)
CFLAGS     =-std=c++17 -Og -g

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
	$(O)/rendergl/glad.o \
	$(O)/rendergl/imgui_impl_sdl2.o \
	$(O)/rendergl/imgui_impl_opengl3.o \
	$(O)/rendergl/imgui.o \
	$(O)/rendergl/imgui_draw.o \
	$(O)/rendergl/imgui_widgets.o \
	$(O)/rendergl/imgui_tables.o
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
	\
	$(O)/allocator/z_heap.o \
	$(O)/allocator/z_alloc.o

all: $(EXE)

$(O)/rendergl/r_framebuffer.o: $(SDIR)/src/r_framebuffer.cpp
	$(COMPILE_SRC)
$(O)/rendergl/r_opengl.o: $(SDIR)/src/r_opengl.cpp
	$(COMPILE_SRC)
$(O)/rendergl/r_vertexcache.o: $(SDIR)/src/r_vertexcache.cpp
	$(COMPILE_SRC)
$(O)/rendergl/r_texture.o: $(SDIR)/src/r_texture.cpp
	$(COMPILE_SRC)
$(O)/rendergl/r_shader.o: $(SDIR)/src/r_shader.cpp
	$(COMPILE_SRC)
$(O)/rendergl/r_spritesheet.o: $(SDIR)/src/r_spritesheet.cpp
	$(COMPILE_SRC)
$(O)/rendergl/m_renderer.o: $(SDIR)/src/m_renderer.cpp
	$(COMPILE_SRC)
$(O)/rendergl/glad.o: $(SDIR)/src/glad.c
	$(COMPILE_C)
$(O)/rendergl/imgui_impl_sdl2.o: $(SDIR)/src/imgui_impl_sdl2.cpp
	$(COMPILE_SRC)
$(O)/rendergl/imgui_impl_opengl3.o: $(SDIR)/src/imgui_impl_opengl3.cpp
	$(COMPILE_SRC)
$(O)/rendergl/imgui.o: $(SDIR)/src/imgui.cpp
	$(COMPILE_SRC)
$(O)/rendergl/imgui_draw.o: $(SDIR)/src/imgui_draw.cpp
	$(COMPILE_SRC)
$(O)/rendergl/imgui_widgets.o: $(SDIR)/src/imgui_widgets.cpp
	$(COMPILE_SRC)
$(O)/rendergl/imgui_tables.o: $(SDIR)/src/imgui_tables.cpp
	$(COMPILE_SRC)
$(O)/common/vm_run.o: $(SDIR)/common/vm_run.cpp
	$(COMPILE_SRC)
$(O)/common/vm.o: $(SDIR)/common/vm.c
	$(COMPILE_C)
$(O)/game/g_syscalls.o: $(SDIR)/src/g_syscalls.cpp
	$(COMPILE_SRC)
$(O)/game/g_sound.o: $(SDIR)/src/g_sound.cpp
	$(COMPILE_SRC)
$(O)/game/g_rng.o: $(SDIR)/src/g_rng.c
	$(COMPILE_SRC)
$(O)/game/g_math.o: $(SDIR)/src/g_math.cpp
	$(COMPILE_SRC)
$(O)/game/g_main.o: $(SDIR)/src/g_main.cpp
	$(COMPILE_SRC)
$(O)/game/g_init.o: $(SDIR)/src/g_init.cpp
	$(COMPILE_SRC)
$(O)/game/g_bff.o: $(SDIR)/src/g_bff.cpp
	$(COMPILE_SRC)
$(O)/game/g_game.o: $(SDIR)/src/g_game.cpp
	$(COMPILE_SRC)
$(O)/engine/n_console.o: $(SDIR)/src/n_console.cpp
	$(COMPILE_SRC)
$(O)/engine/n_scf.o: $(SDIR)/src/n_scf.cpp
	$(COMPILE_SRC)
$(O)/engine/n_shared.o: $(SDIR)/src/n_shared.cpp
	$(COMPILE_SRC)
$(O)/engine/n_common.o: $(SDIR)/src/n_common.cpp
	$(COMPILE_SRC)
$(O)/engine/n_files.o: $(SDIR)/src/n_files.cpp
	$(COMPILE_SRC)
$(O)/allocator/z_heap.o: $(SDIR)/src/z_heap.cpp
	$(COMPILE_SRC)
$(O)/allocator/z_alloc.o: $(SDIR)/src/z_alloc.cpp
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