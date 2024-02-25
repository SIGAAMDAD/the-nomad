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

DEFINES       =$(VERSION_DEFINE) $(DEBUGDEF) -DGDR_DLLCOMPILE -D_NOMAD_ENGINE
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

CC       =$(COMPILER) -std=c++17
CFLAGS   =$(FTYPE) $(DEFINES) $(INCLUDE) $(OPTIMIZERS)
O        =obj
SDIR     =code
LIB      =thenomad_module_lib_x86-64.$(DLL_EXT)

all: $(LIB)

AS_OBJS=\
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

INTERNAL_OBJS=\
	$(O)/engine/n_shared.o \
	$(O)/engine/n_math.o \
	$(O)/module_lib/module_memory.o \
	$(O)/module_lib/module_main.o \
	$(O)/module_lib/module_parse.o \
	$(O)/module_lib/module_handle.o \
	$(O)/module_lib/debugger.o \
	$(O)/module_lib/scriptarray.o \
	$(O)/module_lib/scriptbuilder.o \
	$(O)/module_lib/scriptdictionary.o \
	$(O)/module_lib/scriptstdstring.o \
	$(O)/module_lib/scriptmath.o \

$(O)/module_lib/%.o: $(SDIR)/engine/%.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ -c $<
$(O)/module_lib/%.o: $(SDIR)/module_lib/%.cpp
	$(CC) $(CFLAGS) -shared -fPIC -o $@ -c $<
$(O)/angelscript/%.o: $(SDIR)/angelscript/%.cpp
	$(CC) $(CFLAGS) -shared -fPIC -o $@ -c $<

$(LIB): $(INTERNAL_OBJS) $(AS_OBJS)
	$(CC) $(CFLAGS) $(INTERNAL_OBJS) $(AS_OBJS) -shared -fPIC -o $(LIB)

clean:
	rm -rf $(INTERNAL_OBJS)
	rm $(LIB)
