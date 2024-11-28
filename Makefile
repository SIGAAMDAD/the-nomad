COMPILE_ARCH=$(shell uname -m | sed -e 's/i.86/x86/' | sed -e 's/^arm.*/arm/')
COMPILE_PLATFORM=$(shell uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')

ifndef release
DEBUGDEF  =-D_NOMAD_DEBUG
ifdef win32
FTYPE     =-Og -g
else
FTYPE     =-Og -g
endif
else
DEBUGDEF  =
FTYPE     =-Ofast -g
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
ifeq ($(release),1)
EXE		  =TheNomad.x64
else
EXE       =TheNomad.x64.debug
endif
else
OS_INCLUDE=-I/usr/x86_64-w64-mingw32/include/
COMPILER  =distcc x86_64-w64-mingw32-g++
LIB_PREFIX=dependencies/libs/windows
DLL_EXT   =dll
ifdef release
EXE		  =TheNomad.x64.exe
else
EXE       =TheNomad.x64.debug.exe
endif
endif

VERSION_MAJOR = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0
VERSION_STRING= $(VERSION_MAJOR).$(VERSION_UPDATE).$(VERSION_PATCH)

USE_OPENGL_API=1
USE_VULKAN_API=0

INCLUDE       =-Idependencies/include/ -Idependencies/include/EA/ -Ideps/squirrel/include -Ideps/squall/ $(OS_INCLUDE) \
	-I. -Icode/ -I/usr/local/mono-2.0 -Ideps/ -I/usr/include/PhysX
VERSION_DEFINE=-D_NOMAD_VERSION_MAJOR=$(VERSION_MAJOR) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)
ERRORS        =-Werror=return-type

DEFINES       =$(VERSION_DEFINE) $(DEBUGDEF) -D_NOMAD_ENGINE -DUSE_FMOD
OPTIMIZERS    =\
			-ffast-math \
			-mfma -msse3 -msse2 -msse -mavx2 -mavx \
			-fno-omit-frame-pointer \
			-ftree-vectorize \
			-finline-functions \
			-finline-small-functions \

CFLAGS        =$(FTYPE) -Wno-unused-result $(DEFINES) $(INCLUDE) $(OPTIMIZERS)
ifdef win32
CFLAGS+=-Wno-unused-function -Wno-format -Wno-unused-variable
endif
ifndef release
CFLAGS        +=-Wall -Werror=maybe-uninitialized
endif
CC            =$(COMPILER)
ifdef win32
O             = bin/obj/win64
CFLAGS       += -Icode/libsdl/include/
else
O             = bin/obj/unix
endif
QVM           = qvm
SDIR          = code
VERSION_CC    =-std=c++17 -std=gnu++17
COMPILE_SRC   =$(CC) $(CFLAGS) -o $@ -c $<
COMPILE_C     =distcc gcc $(CFLAGS) -o $@ -c $<
COMPILE_LIBSRC=$(CC) -fPIC -shared $(CFLAGS) -o $@ -c $<

ifeq ($(VM_CANNOT_COMPILE),true)
	DEFINES+=-DVM_CANNOT_COMPILE
endif

ifdef USE_OPENGL_API
DEFINES += -DUSE_OPENGL_API
endif

ifdef USE_VULKAN_API
DEFINES += -DUSE_VULKAN_API
endif

ifndef win32
LDLIBS= \
		-lGL \
		libbacktrace.a \
		libEASTL.a \
		TheNomad.ASLib.x64.a \
		-L. \
		-lSDL2 \
		-lsndfile \
		-lz \
		-lbz2 \
		-lzip \
		-lSDL2_image \
		-Wl,-rpath='.' \
		-ljpeg -lsteam_api \
		-lfmodL -lfmodstudioL \
		-lcurl

ifndef release
LDLIBS+=-leasy_profiler
endif

SYS=\
	$(O)/sys/unix_main.o \
	$(O)/sys/unix_shared.o \
	$(O)/sys/unix_signals.o \
	$(O)/module_lib/module_virtual_asm_linux.o
SYS_DIR=$(SDIR)/unix
else
INCLUDE+=-Ideps/ -Ideps/glm -Icode/libsdl/include -Icode/curl/include/
LDLIBS=-L. \
		-lgdi32 \
		-lmingw32 \
		-lwinmm \
		-lcomctl32 \
		-limagehlp \
		-lpsapi \
		-Wl,-rpath='.' \
		libEASTL.lib \
		-lmsvcrt \
		-lvcruntime140d \
		-lvcruntime140_1d \
		-lSDL2 \
		-lSDL2_image \
		-lopengl32 \
		/usr/x86_64-w64-mingw32/lib/libmsvcrt.a \
		TheNomad.ASLib.x64.lib \
		-Wl,-rpath='.' \
		-ljpeg -lsteam_api64 \
		-lcurl \
		-lfmod \
		-lfmodstudio \

ifndef release
LDLIBS+=-ldbghelp
endif

SYS=\
	$(O)/sys/win_main.o \
	$(O)/sys/win_shared.o \
	$(O)/sys/win_syscon.o \
	$(O)/module_lib/module_virtual_asm_windows.o

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
	$(O)/game/g_screen.o \
	$(O)/game/g_console.o \
	$(O)/game/g_archive.o \
	$(O)/game/g_imgui.o \
	$(O)/game/g_world.o \
	$(O)/game/g_jpeg.o \
	$(O)/game/g_threads.o \
	\
	$(O)/sound/snd_main.o \
	$(O)/sound/snd_bank.o \
	$(O)/sound/snd_world.o \
	\
	$(O)/module_lib/module_memory.o \
	$(O)/module_lib/module_main.o \
	$(O)/module_lib/module_handle.o \
	$(O)/module_lib/module_renderlib.o \
	$(O)/module_lib/module_funcdefs.o \
	$(O)/module_lib/module_jit.o \
	$(O)/module_lib/module_virtual_asm_x64.o \
	$(O)/module_lib/module_debugger.o \
	$(O)/module_lib/scriptbuilder.o \
	$(O)/module_lib/scriptpreprocessor.o \
	$(O)/module_lib/scriptarray.o \
	$(O)/module_lib/scriptany.o \
	$(O)/module_lib/scriptstdstring.o \
	$(O)/module_lib/scriptdictionary.o \
	$(O)/module_lib/scriptstdstring_utils.o \
	$(O)/module_lib/scriptmath.o \
	$(O)/module_lib/scripthandle.o \
	$(O)/module_lib/scriptjson.o \
	$(O)/module_lib/scriptparser.o \
	$(O)/module_lib/contextmgr.o \
	$(O)/module_lib/imgui_stdlib.o \
	$(O)/module_lib/funcdefs/module_funcdef_sound.o \
	$(O)/module_lib/funcdefs/module_funcdef_game.o \
	\
	$(O)/engine/n_common.o \
	$(O)/engine/n_files.o \
	$(O)/engine/n_shared.o \
	$(O)/engine/n_cmd.o \
	$(O)/engine/n_cvar.o \
	$(O)/engine/n_history.o \
	$(O)/engine/n_math.o \
	$(O)/engine/n_memory.o \
	$(O)/engine/n_debug.o \
	$(O)/engine/n_steam.o \
	$(O)/engine/md4.o \
	$(O)/engine/md5.o \
	$(O)/engine/decompress.o \
	$(O)/engine/unzip.o \
	\
	$(O)/rendercommon/imgui.o \
	$(O)/rendercommon/imgui_draw.o \
	$(O)/rendercommon/imgui_widgets.o \
	$(O)/rendercommon/imgui_tables.o \
	$(O)/rendercommon/imgui_impl_sdl2.o \
	$(O)/rendercommon/imgui_impl_opengl3.o \
	\
	$(O)/ui/ui_confirm.o \
	$(O)/ui/ui_main.o \
	$(O)/ui/ui_lib.o \
	$(O)/ui/ui_menu.o \
	$(O)/ui/ui_string_manager.o \
	$(O)/ui/ui_demo.o \
	$(O)/ui/ui_settings.o \
	$(O)/ui/ui_main_menu.o \
	$(O)/ui/ui_play.o \
	$(O)/ui/ui_pause.o \
	$(O)/ui/ui_mods.o \
	$(O)/ui/ui_database.o \
	\
	$(O)/sdl/sdl_input.o \
	$(O)/sdl/sdl_glimp.o \

ifdef build_steam
CFLAGS+=-DNOMAD_STEAM_APP
LDLIBS+= -Wl,-rpath="." -lsteam_api
endif

MAKE=make

default:
	$(MAKE) targets

all: default

MKDIR=mkdir -p

makedirs:
	@if [ ! -d $(O) ];then mkdir $(O);fi
	@if [ ! -d $(O)/game ];then $(MKDIR) $(O)/game;fi
	@if [ ! -d $(O)/sound ];then $(MKDIR) $(O)/sound;fi
	@if [ ! -d $(O)/engine ];then $(MKDIR) $(O)/engine;fi
	@if [ ! -d $(O)/rendercommon ];then $(MKDIR) $(O)/rendercommon;fi
	@if [ ! -d $(O)/sys ];then $(MKDIR) $(O)/sys;fi
	@if [ ! -d $(O)/ui ];then $(MKDIR) $(O)/ui;fi
	@if [ ! -d $(O)/module_lib/ ];then $(MKDIR) $(O)/module_lib;fi
	@if [ ! -d $(O)/angelscript/ ];then $(MKDIR) $(O)/angelscript;fi
	@if [ ! -d $(O)/libjpeg/ ];then $(MKDIR) $(O)/libjpeg;fi
	@if [ ! -d $(O)/sdl ];then $(MKDIR) $(O)/sdl;fi
	@if [ ! -d $(O)/module_lib/funcdefs ];then $(MKDIR) $(O)/module_lib/funcdefs;fi

targets: makedirs
	@echo ""
	@echo "Building $(EXE):"
	@echo ""
	@echo "  VERSION: $(VERSION_MAJOR).$(VERSION_UPDATE).$(VERSION_PATCH)"
	@echo "  PLATFORM: $(PLATFORM)"
	@echo "  ARCH: $(ARCH)"
	@echo "  COMPILE_PLATFORM: $(COMPILE_PLATFORM)"
	@echo "  COMPILE_ARCH: $(COMPILE_ARCH)"
ifdef MINGW
	@echo "  WINDRES: $(WINDRES)"
endif
	@echo "  CC: $(CC)"
	@echo ""
	@echo "  CFLAGS:"
	@for i in $(CFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo "  Output:"
	@for i in $(TARGETS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	$(MAKE) $(EXE)

$(O)/rendercommon/%.o: $(SDIR)/rendercommon/%.cpp
	$(COMPILE_SRC) $(VERSION_CC)
$(O)/game/%.o: $(SDIR)/game/%.cpp
	$(COMPILE_SRC) $(VERSION_CC)
$(O)/sound/%.o: $(SDIR)/sound/%.cpp
	$(COMPILE_SRC) $(VERSION_CC)
$(O)/game/%.o: $(SDIR)/game/%.c
	$(COMPILE_SRC) $(VERSION_CC)
$(O)/engine/%.o: $(SDIR)/engine/%.cpp
	$(COMPILE_SRC) $(VERSION_CC)
$(O)/engine/%.o: $(SDIR)/system/%.cpp
	$(COMPILE_SRC) $(VERSION_CC)
$(O)/engine/%.o: $(SDIR)/engine/%.c
	$(COMPILE_SRC) $(VERSION_CC)
$(O)/ui/%.o: $(SDIR)/ui/%.cpp
	$(COMPILE_SRC) $(VERSION_CC)
$(O)/ui/%.o: $(SDIR)/ui/menulib/%.cpp
	$(COMPILE_SRC) $(VERSION_CC)
$(O)/sys/%.o: $(SYS_DIR)/%.cpp
	$(COMPILE_SRC) $(VERSION_CC)
$(O)/sdl/%.o: $(SDIR)/sdl/%.cpp
	$(COMPILE_SRC) $(VERSION_CC)
$(O)/module_lib/%.o: $(SDIR)/module_lib/%.cpp
	$(COMPILE_SRC) $(VERSION_CC) -DMODULE_LIB
$(O)/module_lib/funcdefs/%.o: $(SDIR)/module_lib/funcdefs/%.cpp
	$(COMPILE_SRC) $(VERSION_CC) -DMODULE_LIB
$(O)/module_lib/%.o: $(SDIR)/module_lib/scriptlib/%.cpp
	$(COMPILE_SRC) $(VERSION_CC) -DMODULE_LIB
$(O)/angelscript/%.o: $(SDIR)/angelscript/%.cpp
ifndef release
	$(COMPILE_SRC) -DMODULE_LIB -DAS_DEBUG
else
	$(COMPILE_SRC) -DMODULE_LIB
endif
$(O)/libjpeg/%.o: $(SDIR)/libjpeg/%.c
	$(COMPILE_C)

ifdef win32
ADD=-flinker-output=exec
endif

$(EXE): $(SRC) $(SYS) $(ASOBJS) $(JPGOBJ)
	$(CC) $(CFLAGS) $(SRC) $(COMMON) $(SYS) $(JPGOBJ) $(ASOBJS) -o $(EXE) $(LDLIBS)

clean.pch:
	rm $(SDIR)/engine/n_pch_all.h.gch
clean.exe:
	rm $(EXE)
clean.all:
	rm -rf $(O)
	rm $(EXE)
clean:
	rm -rf $(SRC) $(SYS)