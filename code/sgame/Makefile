# only to be used for development builds of the vm, used for debugging purposes
CC			= distcc gcc
CFLAGS		= -Og -g -shared -fPIC
INCLUDE		= -I.
COMPILE		= $(CC) $(CFLAGS) $(INCLUDE) -DSGAME -D_NOMAD_VERSION=0 -D_NOMAD_VERSION_UPDATE=1 -D_NOMAD_VERSION_PATCH=1 -o $@ -c $<
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
	$(O)/n_math.o \
	$(O)/n_shared.o \

$(O)/%.o: $(SDIR)/sgame/%.c
	$(COMPILE)
$(O)/%.o: $(SDIR)/engine/%.c
	$(COMPILE)

$(LIB): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(LIB)