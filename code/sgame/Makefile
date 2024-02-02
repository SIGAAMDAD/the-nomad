# only to be used for development builds of the vm, used for debugging purposes
ifndef win32
CC			= distcc gcc
else
CC			= distcc x86_64-w64-mingw32-gcc
endif

CFLAGS		= -Ofast -shared -fPIC -std=c99 -g -Og
INCLUDE		= -I.
COMPILE		= $(CC) $(CFLAGS) $(INCLUDE) -DSGAME -DSGAME_HARD_LINKED -D_NOMAD_VERSION=1 -D_NOMAD_VERSION_UPDATE=1 -D_NOMAD_VERSION_PATCH=0 -o $@ -c $< \
			-Werror=implicit-function-declaration -Werror=incompatible-pointer-types -Wpedantic
O			= obj/sgame
SDIR		= code/

ifdef win32
LIB			= sgamex64.dll
else
LIB			= sgamex86-64.so
endif

.PHONY: all clean

all: $(LIB)

OBJS=\
	$(O)/sg_syscalls.o \
	$(O)/sg_main.o \
	$(O)/sg_mem.o \
	$(O)/sg_level.o \
	$(O)/sg_entity.o \
	$(O)/sg_info.o \
	$(O)/sg_playr.o \
	$(O)/sg_mthink.o \
	$(O)/sg_archive.o \
	$(O)/sg_cmds.o \
	$(O)/sg_gameinfo.o \
	$(O)/sg_draw.o \
	$(O)/sg_imgui.o \
	$(O)/sg_math.o \
	$(O)/sg_hud.o \
	$(O)/n_shared.o \

$(O)/%.o: $(SDIR)/sgame/%.c
	$(COMPILE)
$(O)/%.o: $(SDIR)/engine/%.c
	$(COMPILE)

$(LIB): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(LIB)

clean:
	rm -rf $(O)
