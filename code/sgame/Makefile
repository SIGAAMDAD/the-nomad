# only to be used for development builds of the vm, used for debugging purposes
CC			= distcc gcc
CFLAGS		= -Og -g -shared -fPIC
INCLUDE		= -I.
COMPILE		= $(CC) $(CFLAGS) $(INCLUDE) -DSGAME -DSGAME_HARD_LINKED -D_NOMAD_VERSION=1 -D_NOMAD_VERSION_UPDATE=1 -D_NOMAD_VERSION_PATCH=0 -o $@ -c $< \
			-Werror=implicit-function-declaration -Werror=incompatible-pointer-types
O			= obj/sgame
SDIR		= code/

ifdef win32
LIB			= sgamex64.dll
else
LIB			= sgamex86-64.so
endif

OBJS=\
	$(O)/sg_syscalls.o \
	$(O)/sg_main.o \
	$(O)/sg_mem.o \
	$(O)/sg_level.o \
	$(O)/n_math.o \
	$(O)/n_shared.o \

$(O)/%.o: $(SDIR)/sgame/%.c
	$(COMPILE)
$(O)/%.o: $(SDIR)/engine/%.c
	$(COMPILE)

$(LIB): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(LIB)