VERSION       = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0
CC            = g++
LDLIBS        = /usr/local/lib/libSDL2.a -lSDL2_image /usr/local/lib/libSDL2_ttf.a /usr/local/lib/libopenal.a \
				-lGL libEASTL.a -logg -lvorbisfile -lsndfile
O             = obj
SDIR          = src

.PHONY: all clean

ifdef release
CFLAGS= -Ofast -s -std=c++17 -I/usr/include -Ideps -Isrc/
else
CFLAGS= -Og -g -std=c++17 -I/usr/include -Ideps -Wall -Wpedantic -D_NOMAD_DEBUG -Isrc/
endif

OPIMTIZERS=-fexpensive-optimizations -funroll-loops -ffast-math
DEFINES    =-D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)
CFLAGS    +=$(DEFINES) $(OPIMTIZERS)

OBJS= \
	$(O)/m_renderer.o \
	$(O)/g_game.o \
	$(O)/g_init.o \
	$(O)/g_loop.o \
	$(O)/g_sound.o \
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
	$(O)/imgui.o \
	$(O)/imgui_impl_sdl2.o \
	$(O)/imgui_impl_sdlrenderer.o \
	$(O)/imgui_tables.o \
	$(O)/imgui_widgets.o \
	$(O)/imgui_draw.o \

all: glnomad

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