COMPILE_ARCH=$(shell uname -m | sed -e 's/i.86/x86/' | sed -e 's/^arm.*/arm/')
COMPILE_PLATFORM=$(shell uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')

ifndef release
DEBUGDEF  =-D_NOMAD_DEBUG
FTYPE     =-Og -g
else
DEBUGDEF  =
FTYPE     =-Ofast -s
endif

ifeq ($(shell uname -m),arm64)
	COMPILE_ARCH=aarch64
endif
ifeq ($(COMPILE_PLATFORM),mingw32)
	ifeq ($(COMPILE_ARCH),i386)
		COMPILE_ARCH=x86
	endif
endif

ifeq ($(COMPILE_PLATFORM),cygwin)
	PLATFORM=mingw32
endif

ifndef PLATFORM
	PLATFORM=$(COMPILE_PLATFORM)
endif
export PLATFORM

ifeq ($(PLATFORM),mingw32)
	win32=1
endif
ifeq ($(PLATFORM),mingw64)
	win32=1
endif

ifeq ($(COMPILE_ARCH),amd64)
	COMPILE_ARCH=x86_64
endif
ifeq ($(COMPILE_ARCH),x64)
	COMPILE_ARCH=x64
endif

ifndef ARCH
ARCH=$(COMPILE_ARCH)
endif
export ARCH

ifneq ($(PLATFORM),$(COMPILE_PLATFORM))
	CROSS_COMPILING=1
else
	CROSS_COMPILING=0
	ifneq ($(ARCH),$(COMPILE_ARCH))
		CROSS_COMPILING=1
	endif
endif
export CROSS_COMPILING


ifeq ($(ARCH),x86_64)
	VM_CANNOT_COMPILE=false
	VM_FILE=vm_x86
else ifeq ($(ARCH),x86)
	VM_CANNOT_COMPILE=false
	VM_FILE=vm_x86
else
	VM_CANNOT_COMPILE=true
	VM_FILE=
endif

ifeq ($(ARCH),arm)
	VM_CANNOT_COMPILE=false
	VM_FILE=vm_aarch64
endif
ifeq ($(ARCH),aarch64)
	VM_CANNOT_COMPILE=false
	VM_FILE=vm_aarch64
endif

ifndef win32
OS_INCLUDE=-I/usr/include/ -I/usr/local/include/
COMPILER  =distcc g++
LIB_PREFIX=dependencies/libs/linux
DLL_EXT   =so
else
OS_INCLUDE=-I/usr/x86_64-w64-mingw32/include/
COMPILER  =distcc x86_64-w64-mingw32-g++
LIB_PREFIX=dependencies/libs/windows
DLL_EXT   =dll
endif

VERSION       = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0

INCLUDE       =-Idependencies/include/ -Idependencies/include/EA/ $(OS_INCLUDE) -I. -Icode/
VERSION_DEFINE=-D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)

DEFINES       =$(VERSION_DEFINE) $(DEBUGDEF)
OPTIMIZERS    =\
			-ffast-math \
			-rdynamic -export-dynamic \
			-mfma -msse3 -msse2 -msse -mavx -mavx2 -mmmx -mfpmath=sse

CFLAGS        =-std=c++17 $(FTYPE) $(DEFINES) $(INCLUDE) $(OPTIMIZERS)
CC            =$(COMPILER)
O             = obj
QVM           = qvm
SDIR          = code
EXE           = glnomad
COMPILE_SRC   =$(CC) $(CFLAGS) -o $@ -c $<
COMPILE_LIBSRC=$(CC) -fPIC -shared $(CFLAGS) -o $@ -c $<

ifeq ($(VM_CANNOT_COMPILE),true)
	DEFINES+=-DVM_CANNOT_COMPILE
endif

ifndef win32
LDLIBS=-L/usr/lib/x86_64-linux-gnu/ \
		-lz \
		-lbz2 \
		-lGL \
		-lboost_thread \
		$(LIB_PREFIX)/libEASTL.a \
		$(LIB_PREFIX)/libzstd.a \
		$(LIB_PREFIX)/libopenal.a \
		-L$(LIB_PREFIX) \
		-lxallocator \
		-lSDL2 \
		-lSDL2_image \
		-lSDL2_ttf \
		-lsndfile \
		-lglad \
		-leasy_profiler
SYS=\
	$(O)/sys/unix_main.o
SYS_DIR=$(SDIR)/unix
else
LDLIBS=-L$(LIB_PREFIX) \
		-lSDL2 \
		-lz \
		-lbz2 \
		-lSDL2_image \
		-lSDL2_ttf \
		-lopenal \
		-lGL \
		-lsndfile \
		-lxallocator \
		-lzstd \
		-lsteam_api \
		-lboost_thread \
		-lglad \
		-leasy_profiler
SYS=\
	$(O)/sys/win_main.o
SYS_DIR=$(SDIR)/win32
endif

.PHONY: all clean targets clean.objs clean.exe clean.pch pch makedirs default

COMMON_DIR=$(O)/common
ENGINE_DIR=$(O)/engine
ALLOCATOR_DIR=$(O)/allocator
GAME_DIR=$(O)/game

COMPILE_SRC=$(CC) $(CFLAGS) -o $@ -c $<

COMMON=\
	$(O)/common/vm.o \
	$(O)/common/vm_interpreted.o \
	$(O)/common/vm_x86.o
SRC=\
	$(O)/game/g_rng.o \
	$(O)/game/g_init.o \
	$(O)/game/g_bff.o \
	$(O)/game/g_game.o \
	$(O)/game/g_sgame.o \
	\
	$(O)/engine/n_console.o \
	$(O)/engine/n_scf.o \
	$(O)/engine/n_common.o \
	$(O)/engine/n_files.o \
	$(O)/engine/n_map.o \
	$(O)/engine/n_shared.o \
	$(O)/engine/n_cmd.o \
	$(O)/engine/n_cvar.o \
	$(O)/engine/n_history.o \
	$(O)/engine/n_event.o \
	$(O)/engine/n_sound.o \
	$(O)/engine/md4.o \
	\
	$(O)/allocator/z_heap.o \
	$(O)/allocator/z_hunk.o \
	$(O)/allocator/z_zone.o \
	$(O)/allocator/z_alloc.o \


MAKE=make

default:
	$(MAKE) targets

all: default

MKDIR=mkdir -p

makedirs:
	@if [ ! -d $(O) ];then mkdir $(O);fi
	@if [ ! -d $(O)/game ];then $(MKDIR) $(O)/game;fi
	@if [ ! -d $(O)/allocator ];then $(MKDIR) $(O)/allocator;fi
	@if [ ! -d $(O)/common ];then $(MKDIR) $(O)/common;fi
	@if [ ! -d $(O)/engine ];then $(MKDIR) $(O)/engine;fi
	@if [ ! -d $(O)/sys ];then $(MKDIR) $(O)/sys;fi
	@if [ ! -d $(O)/nmap ];then $(MKDIR) $(O)/nmap;fi

targets: makedirs
	$(MAKE) $(EXE)

$(O)/common/%.o: $(SDIR)/common/%.cpp
	$(COMPILE_SRC)
$(O)/common/%.o: $(SDIR)/common/%.c
	$(COMPILE_SRC)
$(O)/game/%.o: $(SDIR)/game/%.cpp
	$(COMPILE_SRC)
$(O)/game/%.o: $(SDIR)/game/%.c
	$(COMPILE_SRC)
$(O)/engine/%.o: $(SDIR)/engine/%.cpp
	$(COMPILE_SRC)
$(O)/engine/%.o: $(SDIR)/engine/%.c
	$(COMPILE_SRC)
$(O)/allocator/%.o: $(SDIR)/allocator/%.cpp
	$(COMPILE_SRC)
$(O)/sys/%.o: $(SYS_DIR)/%.cpp
	$(COMPILE_SRC)

$(EXE): $(SRC) $(COMMON) $(SYS)
	$(CC) $(CFLAGS) $(SRC) $(COMMON) $(SYS) -o $(EXE) $(LDLIBS) -Wl,-rpath=. rendergl.$(DLL_EXT)

clean.pch:
	rm $(SDIR)/src/n_pch_all.h.gch
clean.exe:
	rm $(EXE)
clean.objs:
	rm -rf $(O)
clean:
	rm -rf $(O)
	rm $(EXE)
