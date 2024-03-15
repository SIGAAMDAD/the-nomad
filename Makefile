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
EXE		  =TheNomad
else
OS_INCLUDE=-I/usr/x86_64-w64-mingw32/include/
COMPILER  =distcc x86_64-w64-mingw32-g++
LIB_PREFIX=dependencies/libs/windows
DLL_EXT   =dll
EXE		  =TheNomad.exe
endif

VERSION       = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0
VERSION_STRING= $(VERSION).$(VERSION_UPDATE).$(VERSION_PATCH)

USE_OPENGL_API=1
USE_VULKAN_API=0

INCLUDE       =-Idependencies/include/ -Idependencies/include/EA/ -Ideps/squirrel/include -Ideps/squall/ $(OS_INCLUDE) -I. -Icode/
VERSION_DEFINE=-D_NOMAD_VERSION=$(VERSION) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)

DEFINES       =$(VERSION_DEFINE) $(DEBUGDEF) -D_NOMAD_ENGINE -DAS_MAX_PORTABILITY
OPTIMIZERS    =\
			-ffast-math \
			-mfma -msse3 -msse2 -msse -mavx -mavx2 -mmmx -mfpmath=sse

CFLAGS        =-std=c++17 $(FTYPE) -Wno-unused-result $(DEFINES) $(INCLUDE) $(OPTIMIZERS)
CC            =$(COMPILER)
O             = obj
QVM           = qvm
SDIR          = code
COMPILE_SRC   =$(CC) $(CFLAGS) -o $@ -c $<
COMPILE_C     =distcc gcc $(FTYPE) $(OPTIMIZERS) $(INCLUDE) -o $@ -c $<
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
LDLIBS=-L/usr/lib/x86_64-linux-gnu/ \
		-lGL \
		-lbacktrace \
		$(LIB_PREFIX)/libEASTL.a \
		$(LIB_PREFIX)/libopenal.a \
		-L$(LIB_PREFIX) \
		-lSDL2 \
		-lsndfile \
#		-leasy_profiler
SYS=\
	$(O)/sys/unix_main.o \
	$(O)/sys/unix_shared.o \
	$(O)/module_lib/module_virtual_asm_linux.o
SYS_DIR=$(SDIR)/unix
else
INCLUDE+=-Ideps/
LDLIBS=-L. \
		-lSDL2 \
		-lOpenAL32 \
		-lopengl32 \
		-lsndfile \
		-lgdi32 \
		-lmingw32 \
		-lwinmm \
		-lcomctl32 \
		-limagehlp \
		-lpsapi \
		-ljpeg-9 \
		-lEASTL \
		/usr/x86_64-w64-mingw32/lib/libmsvcrt.a \
		-static-libgcc -static-libstdc++ \

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

ifndef win32 
JPGOBJ = \
  	$(O)/game/jpeg/jaricom.o \
  	$(O)/game/jpeg/jcapimin.o \
  	$(O)/game/jpeg/jcapistd.o \
  	$(O)/game/jpeg/jcarith.o \
  	$(O)/game/jpeg/jccoefct.o  \
  	$(O)/game/jpeg/jccolor.o \
  	$(O)/game/jpeg/jcdctmgr.o \
  	$(O)/game/jpeg/jchuff.o   \
  	$(O)/game/jpeg/jcinit.o \
  	$(O)/game/jpeg/jcmainct.o \
  	$(O)/game/jpeg/jcmarker.o \
  	$(O)/game/jpeg/jcmaster.o \
  	$(O)/game/jpeg/jcomapi.o \
  	$(O)/game/jpeg/jcparam.o \
  	$(O)/game/jpeg/jcprepct.o \
  	$(O)/game/jpeg/jcsample.o \
  	$(O)/game/jpeg/jctrans.o \
  	$(O)/game/jpeg/jdapimin.o \
  	$(O)/game/jpeg/jdapistd.o \
  	$(O)/game/jpeg/jdarith.o \
  	$(O)/game/jpeg/jdatadst.o \
  	$(O)/game/jpeg/jdatasrc.o \
  	$(O)/game/jpeg/jdcoefct.o \
  	$(O)/game/jpeg/jdcolor.o \
  	$(O)/game/jpeg/jddctmgr.o \
  	$(O)/game/jpeg/jdhuff.o \
  	$(O)/game/jpeg/jdinput.o \
  	$(O)/game/jpeg/jdmainct.o \
  	$(O)/game/jpeg/jdmarker.o \
  	$(O)/game/jpeg/jdmaster.o \
  	$(O)/game/jpeg/jdmerge.o \
  	$(O)/game/jpeg/jdpostct.o \
  	$(O)/game/jpeg/jdsample.o \
  	$(O)/game/jpeg/jdtrans.o \
  	$(O)/game/jpeg/jerror.o \
  	$(O)/game/jpeg/jfdctflt.o \
  	$(O)/game/jpeg/jfdctfst.o \
  	$(O)/game/jpeg/jfdctint.o \
  	$(O)/game/jpeg/jidctflt.o \
  	$(O)/game/jpeg/jidctfst.o \
  	$(O)/game/jpeg/jidctint.o \
  	$(O)/game/jpeg/jmemmgr.o \
  	$(O)/game/jpeg/jmemnobs.o \
  	$(O)/game/jpeg/jquant1.o \
  	$(O)/game/jpeg/jquant2.o \
  	$(O)/game/jpeg/jutils.o
endif

ASOBJS=\
	$(O)/angelscript/as_atomic.o \
	$(O)/angelscript/as_builder.o \
	$(O)/angelscript/as_bytecode.o \
	$(O)/angelscript/as_callfunc_x86.o \
	$(O)/angelscript/as_callfunc.o \
	$(O)/angelscript/as_compiler.o \
	$(O)/angelscript/as_configgroup.o \
	$(O)/angelscript/as_context.o \
	$(O)/angelscript/as_datatype.o \
	$(O)/angelscript/as_gc.o \
	$(O)/angelscript/as_generic.o \
	$(O)/angelscript/as_globalproperty.o \
	$(O)/angelscript/as_memory.o \
	$(O)/angelscript/as_module.o \
	$(O)/angelscript/as_objecttype.o \
	$(O)/angelscript/as_outputbuffer.o \
	$(O)/angelscript/as_parser.o \
	$(O)/angelscript/as_restore.o \
	$(O)/angelscript/as_scriptcode.o \
	$(O)/angelscript/as_scriptengine.o \
	$(O)/angelscript/as_scriptfunction.o \
	$(O)/angelscript/as_scriptnode.o \
	$(O)/angelscript/as_scriptobject.o \
	$(O)/angelscript/as_string_util.o \
	$(O)/angelscript/as_string.o \
	$(O)/angelscript/as_thread.o \
	$(O)/angelscript/as_tokenizer.o \
	$(O)/angelscript/as_typeinfo.o \
	$(O)/angelscript/as_variablescope.o \

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
	$(O)/game/g_world.o \
	$(O)/game/g_jpeg.o \
	\
	$(O)/module_lib/module_memory.o \
	$(O)/module_lib/module_main.o \
	$(O)/module_lib/module_handle.o \
	$(O)/module_lib/module_renderlib.o \
	$(O)/module_lib/module_funcdefs.o \
	$(O)/module_lib/module_jit.o \
	$(O)/module_lib/module_virtual_asm_x64.o \
	$(O)/module_lib/debugger.o \
	$(O)/module_lib/scriptarray.o \
	$(O)/module_lib/scriptbuilder.o \
	$(O)/module_lib/scriptdictionary.o \
	$(O)/module_lib/scriptstdstring.o \
	$(O)/module_lib/scriptstdstring_utils.o \
	$(O)/module_lib/scriptmath.o \
	$(O)/module_lib/scripthandle.o \
	$(O)/module_lib/contextmgr.o \
	$(O)/module_lib/imgui_stdlib.o \
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
	$(O)/ui/ui_string_manager.o \
	$(O)/ui/ui_window.o \
	$(O)/ui/ui_title.o \
	$(O)/ui/ui_settings.o \
	$(O)/ui/ui_intro.o \
	$(O)/ui/ui_main_menu.o \
	$(O)/ui/ui_single_player.o \
	$(O)/ui/ui_pause.o \
	$(O)/ui/ui_legal.o \
	$(O)/ui/ui_mods.o \

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
	@if [ ! -d $(O)/game/jpeg ];then $(MKDIR) $(O)/game/jpeg;fi
	@if [ ! -d $(O)/module_lib/ ];then $(MKDIR) $(O)/module_lib;fi
	@if [ ! -d $(O)/angelscript/ ];then $(MKDIR) $(O)/angelscript;fi

targets: makedirs
	@echo ""
	@echo "Building thenomad:"
	@echo ""
	@echo "  VERSION: $(VERSION)"
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
$(O)/ui/%.o: $(SDIR)/ui/menulib/%.cpp
	$(COMPILE_SRC)
$(O)/sys/%.o: $(SYS_DIR)/%.cpp
	$(COMPILE_SRC)
$(O)/game/jpeg/%.o: $(SDIR)/libjpeg/%.c
	$(COMPILE_C)
$(O)/module_lib/%.o: $(SDIR)/module_lib/%.cpp
	$(COMPILE_SRC) -DMODULE_LIB
$(O)/angelscript/%.o: $(SDIR)/angelscript/%.cpp
	$(COMPILE_SRC)

ifdef win32
ADD=-flinker-output=exec
endif

$(EXE): $(SRC) $(COMMON) $(SYS) $(JPGOBJ) $(ASOBJS)
	$(CC) $(CFLAGS) $(SRC) $(COMMON) $(SYS) $(ADD) $(JPGOBJ) $(ASOBJS) -o $(EXE) $(LDLIBS)

clean.pch:
	rm $(SDIR)/engine/n_pch_all.h.gch
clean.exe:
	rm $(EXE)
clean.all:
	rm -rf $(O)
	rm $(EXE)
clean:
	rm -rf $(O)