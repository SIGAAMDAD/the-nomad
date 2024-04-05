ifndef win32
OS_INCLUDE=-I/usr/include/ -I/usr/local/include/
COMPILER  =distcc gcc
LIB_PREFIX=dependencies/libs/linux
DLL_EXT   =so
O         =bin/obj/unix
else
OS_INCLUDE=-I/usr/x86_64-w64-mingw32/include/
COMPILER  =distcc x86_64-w64-mingw32-gcc
LIB_PREFIX=dependencies/libs/windows
DLL_EXT   =dll
O         =bin/obj/win64
endif

ifndef release
DEBUGDEF  =-D_NOMAD_DEBUG
FTYPE     =-Og -g
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

DEFINES       =$(VERSION_DEFINE) $(DEBUGDEF) -DGDR_DLLCOMPILE -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -D_NOMAD_ENGINE -DHQ_BLUR
OPTIMIZERS    =\
				-fexpensive-optimizations \
				-funroll-loops \
				-ffast-math \
				-mfma -msse3 -msse2 -mno-avx2 -mno-avx
#				-ftree-vectorize \
				-finline-functions \
				-finline-small-functions \

ifeq ($(build),debug)
	OPTIMIZERS +=-fvar-tracking
endif

CC       =$(COMPILER) -std=c99
CFLAGS   =$(FTYPE) $(DEFINES) $(INCLUDE) $(OPTIMIZERS)
SDIR     =code
LIB      =TheNomad.RenderLib-opengl.x64.$(DLL_EXT)

all: $(LIB)

INTERNAL_OBJS=\
	$(O)/rendergl/rgl_cache.o \
	$(O)/rendergl/rgl_scene.o \
	$(O)/rendergl/rgl_backend.o \
	$(O)/rendergl/rgl_shader.o \
	$(O)/rendergl/rgl_cmd.o \
	$(O)/rendergl/rgl_draw.o \
	$(O)/rendergl/rgl_shade_calc.o \
	$(O)/rendergl/rgl_extensions.o \
	$(O)/rendergl/rgl_glsl_fallback.o \
	$(O)/rendergl/rgl_init.o \
	$(O)/rendergl/rgl_program.o \
	$(O)/rendergl/rgl_texture.o \
	$(O)/rendergl/rgl_world.o \
	$(O)/rendergl/rgl_main.o \
	$(O)/rendergl/rgl_fbo.o \
	$(O)/rendergl/rgl_math.o \
	$(O)/rendergl/rgl_image_dds.o \
	$(O)/rendergl/r_image_bmp.o \
	$(O)/rendergl/r_image_jpg.o \
	$(O)/rendergl/r_image_pcx.o \
	$(O)/rendergl/r_image_png.o \
	$(O)/rendergl/r_image_tga.o \
	$(O)/rendergl/r_noise.o \
	$(O)/rendergl/n_shared.o \
	$(O)/rendergl/n_math.o \
	$(O)/rendergl/puff.o \

$(O)/rendergl/%.o: $(SDIR)/engine/%.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ -c $<
$(O)/rendergl/%.o: $(SDIR)/rendergl/%.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ -c $<
$(O)/rendergl/%.o: $(SDIR)/rendercommon/%.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ -c $<

$(LIB): $(INTERNAL_OBJS)
	$(CC) $(CFLAGS) $(INTERNAL_OBJS) -shared -fPIC -o $(LIB) -lm

clean:
	rm -rf $(INTERNAL_OBJS)
