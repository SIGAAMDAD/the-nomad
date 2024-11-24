ifndef win32
OS_INCLUDE=-I/usr/include/ -I/usr/local/include/
COMPILER  =distcc g++ -std=c++17
LIB_PREFIX=dependencies/libs/linux
O         =bin/obj/unix
else
OS_INCLUDE=-I/usr/x86_64-w64-mingw32/include/ -Idependencies/include/ -Ideps/glm/ -Icode/libsdl/include/
COMPILER  =distcc x86_64-w64-mingw32-g++ -std=c++17
LIB_PREFIX=dependencies/libs/windows
O         =bin/obj/win64
endif

ifndef release
DEBUGDEF  =-D_NOMAD_DEBUG
FTYPE     =-Og -g
else
DEBUGDEF  =
FTYPE     =-Ofast -g
endif

VERSION_MAJOR = 1
VERSION_UPDATE= 1
VERSION_PATCH = 0
VERSION_STRING= $(VERSION_MAJOR).$(VERSION_UPDATE).$(VERSION_PATCH)

INCLUDE       =-Idependencies/include/ -Idependencies/include/EA/ $(OS_INCLUDE) -I. -Icode/ -Idependencies/include/jdk
VERSION_DEFINE=-D_NOMAD_VERSION_MAJOR=$(VERSION_MAJOR) -D_NOMAD_VERSION_UPDATE=$(VERSION_UPDATE) -D_NOMAD_VERSION_PATCH=$(VERSION_PATCH)

.PHONY: all clean

DEFINES       =$(VERSION_DEFINE) $(DEBUGDEF) -D_NOMAD_ENGINE -DMODULE_LIB -DasUSE_COMPUTED_GOTOS #-DAS_MAX_PORTABILITY
ifndef release
DEFINES      +=-DAS_DEBUG
endif
OPTIMIZERS    =\
				-fexpensive-optimizations \
				-funroll-loops \
				-ffast-math \
				-mfma -msse3 -msse2 -mavx2 -mavx \
				-ftree-vectorize \
				-finline-functions \
				-finline-small-functions \

CC       =$(COMPILER)
CFLAGS   =$(FTYPE) $(DEFINES) $(INCLUDE) $(OPTIMIZERS) -fPIC
SDIR     =code

ifdef win32
LIB      =TheNomad.ASLib.x64.lib
COMPILE_LIB=x86_64-w64-mingw32-ar
else
LIB      =TheNomad.ASLib.x64.a
COMPILE_LIB=ar
endif

all: $(LIB)

INTERNAL_OBJS=\
	$(O)/angelscript/as_atomic.o \
	$(O)/angelscript/as_builder.o \
	$(O)/angelscript/as_bytecode.o \
	$(O)/angelscript/as_callfunc_x64_gcc.o \
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

$(O)/angelscript/%.o: $(SDIR)/angelscript/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(LIB): $(INTERNAL_OBJS)
	$(COMPILE_LIB) rcs $(LIB) $(INTERNAL_OBJS)

clean:
	rm -rf $(INTERNAL_OBJS)
