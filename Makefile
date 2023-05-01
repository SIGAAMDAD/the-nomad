VERSION       = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0
CC            = distcc g++ -I/usr/include -Isrc/glad/include -I/usr/local/include -Isrc/idlib
LDLIBS        = -fPIE /usr/local/lib/libSDL2.a -lSDL2_image /usr/local/lib/libSDL2_ttf.a /usr/local/lib/libopenal.a \
				-lGL libEASTL.a -logg -lvorbisfile -lsndfile -lbz2 /usr/local/lib/xallocator.a
O             = obj
SDIR          = src

.PHONY: all clean

ifdef release
CFLAGS= -Ofast -s -std=c++17 -Ideps -Isrc/ -Wno-class-memaccess
LDLIBS+=libglad.a
else
CFLAGS= -Og -g -std=c++17 -Ideps -Wall -Wpedantic -D_NOMAD_DEBUG -Isrc/ -Wno-class-memaccess
LDLIBS+=libglad_dbg.a
endif
OPIMTIZERS=-fexpensive-optimizations -funroll-loops -ffast-math -finline-limit=10000 -mavx -mavx2 -mfma -msse3
DEFINES    =-D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)
CFLAGS    +=$(DEFINES) $(OPIMTIZERS)


OBJS= \
	$(O)/m_renderer.o \
	$(O)/g_init.o \
	$(O)/g_loop.o \
	$(O)/g_sound.o \
	$(O)/g_game.o \
	$(O)/g_zone.o \
	$(O)/n_scf.o \
	$(O)/p_playr.o \
	$(O)/s_mmisc.o \
	$(O)/s_mthink.o \
	$(O)/info.o \
	$(O)/g_main.o \
	$(O)/g_rng.o \
	$(O)/s_saveg.o \
	$(O)/g_bff.o \
	$(O)/imgui_draw.o \
	$(O)/imgui_impl_sdl2.o \
	$(O)/imgui_impl_sdlrenderer.o \
	$(O)/imgui.o \
	$(O)/imgui_widgets.o \
	$(O)/imgui_tables.o \
	$(O)/r_opengl.o \
	$(O)/n_shared.o \

all: glnomad

$(O)/%.o: $(SDIR)/idlib/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<
$(O)/s_saveg.o: $(SDIR)/s_saveg.cpp
	$(CC) $(CFLAGS) -Wno-unused-result -o $@ -c $<
$(O)/g_bff.o: $(SDIR)/g_bff.cpp
	$(CC) $(CFLAGS) -Wno-unused-result -o $@ -c $<
$(O)/%.o: $(SDIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

glnomad: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o glnomad $(LDLIBS)

clean:
	rm -r $(OBJS)
	rm glnomad