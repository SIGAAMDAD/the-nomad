VERSION       = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0
CC            = g++
LDLIBS        = /usr/local/lib/libSDL2.a -lSDL2_image /usr/local/lib/libSDL2_ttf.a -lsndfile /usr/local/lib/libopenal.a \
				-lGL
O             = obj
SDIR          = src

.PHONY: all clean

ifdef release
CFLAGS= -Ofast -s -std=c++17 -I/usr/include -Ideps -fno-exceptions
else
CFLAGS= -Og -g -std=c++17 -I/usr/include -Ideps -Wall -Wpedantic -D_NOMAD_DEBUG
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
	$(O)/imgui.o \
	$(O)/imgui_impl_sdl2.o \
	$(O)/imgui_impl_sdlrenderer.o \
	$(O)/imgui_tables.o \
	$(O)/imgui_widgets.o \
	$(O)/imgui_draw.o \

all: glnomad

$(O)/%.o: $(SDIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

glnomad: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o glnomad $(LDLIBS)

clean:
	rm -r $(OBJS)
	rm glnomad