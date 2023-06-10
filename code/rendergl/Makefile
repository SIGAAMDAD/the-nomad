VERSION       =1
VERSION_UPDATE=1
VERSION_PATCH =0
CC            = g++
CFLAGS        = -Og -g -shared -fPIC -std=c++17
INCLUDE       = -I/usr/include \
				-Ideps \
				-Ideps/glad/include \
				-Ideps/imgui \
				-I/usr/local/include \
				-I/usr/include/freetype2 \
				-Isrc
OPIMTIZERS    = -fexpensive-optimizations -funroll-loops -ffast-math -finline-limit=10000 -mfma -mavx2
DEFINES       = -D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH) -DGDR_DLLCOMPILE
O             = obj/rendergl
SDIR          = code/rendergl
LIB           = renderglx64.so
LDFLAGS       = -static -lSDL2

CFLAGS       +=$(INCLUDE)$(OPTIMIZERS) $(DEFINES)

.PHONY: all clean clean.lib clean.objs

all: $(LIB)

OBJS=\
	$(O)/rgl_init.o \
	$(O)/rgl_image.o \
	$(O)/rgl_cache.o \

$(O)/%.o: $(SDIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(LIB): $(OBJS) obj/n_shared.o
	$(CC) $(CFLAGS) $(OBJS) obj/n_shared.o -o $(LIB)