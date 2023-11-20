COMPILE_ARCH=$(shell uname -m | sed -e 's/i.86/x86/' | sed -e 's/^arm.*/arm/')
COMPILE_PLATFORM=$(shell uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')

ifndef release
DEBUGDEF  =-D_NOMAD_DEBUG
ifdef win32
FTYPE     =-Og -g3
else
FTYPE     =-Og -g
endif
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
EXE		  =glnomad
else
OS_INCLUDE=-I/usr/x86_64-w64-mingw32/include/
COMPILER  =distcc x86_64-w64-mingw32-g++
LIB_PREFIX=dependencies/libs/windows
DLL_EXT   =dll
EXE		  =glnomad.exe
endif

VERSION       = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0

INCLUDE       =-Idependencies/include/ -Idependencies/include/EA/ $(OS_INCLUDE) -I. -Icode/
VERSION_DEFINE=-D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)

DEFINES       =$(VERSION_DEFINE) $(DEBUGDEF)
OPTIMIZERS    =\
			-ffast-math \
			-mfma -msse3 -msse2 -msse -mavx -mavx2 -mmmx -mfpmath=sse

CFLAGS        =-std=c++17 $(FTYPE) -Wno-unused-result $(DEFINES) $(INCLUDE) $(OPTIMIZERS)
CC            =$(COMPILER)
O             = obj
QVM           = qvm
SDIR          = code
COMPILE_SRC   =$(CC) $(CFLAGS) -o $@ -c $<
COMPILE_LIBSRC=$(CC) -fPIC -shared $(CFLAGS) -o $@ -c $<

ifeq ($(VM_CANNOT_COMPILE),true)
	DEFINES+=-DVM_CANNOT_COMPILE
endif

ifndef win32
LDLIBS=-L/usr/lib/x86_64-linux-gnu/ \
		-lGL \
		-lbacktrace \
		-lboost_thread \
		$(LIB_PREFIX)/libEASTL.a \
		$(LIB_PREFIX)/libopenal.a \
		-L$(LIB_PREFIX) \
		-lSDL2 \
		-lsndfile \
		-leasy_profiler
SYS=\
	$(O)/sys/unix_main.o \
	$(O)/sys/unix_shared.o
SYS_DIR=$(SDIR)/unix
else
LDLIBS=-L. \
		-lSDL2 \
		-lOpenAL32 \
		-lopengl32 \
		-lsndfile \
		-lboost_thread \
		-lgdi32 \
		-lmingw32 \
		-lwinmm \
		-lcomctl32 \
		-limagehlp \
		-static-libgcc -static-libstdc++

ifndef release
LDLIBS+=-ldbghelp
endif

SYS=\
	$(O)/sys/win_main.o \
	$(O)/sys/win_shared.o \
	$(O)/sys/win_syscon.o \

SYS_DIR=$(SDIR)/win32
INCLUDE+=-Idependencies/include/libsndfile -Idependencies/include/boost -I./mingw32/include
endif

.PHONY: all clean targets clean.objs clean.exe clean.pch pch makedirs default

ENGINE_DIR=$(O)/engine
GAME_DIR=$(O)/game

COMPILE_SRC=$(CC) $(CFLAGS) -o $@ -c $<

SRC=\
	$(O)/game/g_game.o \
	$(O)/game/g_sgame.o \
	$(O)/game/g_ui.o \
	$(O)/game/g_event.o \
	$(O)/game/g_sound.o \
	$(O)/game/g_screen.o \
	$(O)/game/g_console.o \
	$(O)/game/g_archive.o \
	$(O)/game/g_imgui.o \
	\
	$(O)/engine/vm.o \
	$(O)/engine/vm_interpreted.o \
	$(O)/engine/vm_x86.o \
	$(O)/engine/n_common.o \
	$(O)/engine/n_files.o \
	$(O)/engine/n_shared.o \
	$(O)/engine/n_cmd.o \
	$(O)/engine/n_cvar.o \
	$(O)/engine/n_history.o \
	$(O)/engine/n_math.o \
	$(O)/engine/n_memory.o \
	$(O)/engine/n_debug.o \
	$(O)/engine/md4.o \
	\
	$(O)/rendercommon/imgui.o \
	$(O)/rendercommon/imgui_draw.o \
	$(O)/rendercommon/imgui_widgets.o \
	$(O)/rendercommon/imgui_tables.o \
	$(O)/rendercommon/imgui_impl_sdl2.o \
	$(O)/rendercommon/imgui_impl_opengl3.o \
	\
	$(O)/ui/ui_main.o \
	$(O)/ui/ui_lib.o \
	$(O)/ui/ui_menu.o \
	$(O)/ui/ui_mfield.o \
	$(O)/ui/ui_font.o \
	$(O)/ui/ui_string_manager.o \
	$(O)/ui/ui_window.o \
	$(O)/ui/ui_title.o \
	$(O)/ui/ui_settings.o \
	$(O)/ui/ui_intro.o \
	$(O)/ui/ui_main_menu.o \
	$(O)/ui/ui_single_player.o \

MAKE=make

default:
	$(MAKE) targets

all: default

MKDIR=mkdir -p

makedirs:
	@if [ ! -d $(O) ];then mkdir $(O);fi
	@if [ ! -d $(O)/game ];then $(MKDIR) $(O)/game;fi
	@if [ ! -d $(O)/engine ];then $(MKDIR) $(O)/engine;fi
	@if [ ! -d $(O)/rendercommon ];then $(MKDIR) $(O)/rendercommon;fi
	@if [ ! -d $(O)/sys ];then $(MKDIR) $(O)/sys;fi
	@if [ ! -d $(O)/ui ];then $(MKDIR) $(O)/ui;fi

targets: makedirs
	$(MAKE) $(EXE)

$(O)/rendercommon/%.o: $(SDIR)/rendercommon/%.cpp
	$(COMPILE_SRC)
$(O)/game/%.o: $(SDIR)/game/%.cpp
	$(COMPILE_SRC)
$(O)/game/%.o: $(SDIR)/game/%.c
	$(COMPILE_SRC)
$(O)/engine/%.o: $(SDIR)/engine/%.cpp
	$(COMPILE_SRC)
$(O)/engine/%.o: $(SDIR)/system/%.cpp
	$(COMPILE_SRC)
$(O)/engine/%.o: $(SDIR)/engine/%.c
	$(COMPILE_SRC)
$(O)/ui/%.o: $(SDIR)/ui/%.cpp
	$(COMPILE_SRC)
$(O)/sys/%.o: $(SYS_DIR)/%.cpp
	$(COMPILE_SRC)

ifdef win32
ADD=-flinker-output=exec
endif

$(EXE): $(SRC) $(COMMON) $(SYS)
	$(CC) $(CFLAGS) $(SRC) $(COMMON) $(SYS) $(ADD) -o $(EXE) $(LDLIBS)

clean.pch:
	rm $(SDIR)/engine/n_pch_all.h.gch
clean.exe:
	rm $(EXE)
clean.all:
	rm -rf $(O)
	rm $(EXE)
clean:
	rm -rf $(O)
