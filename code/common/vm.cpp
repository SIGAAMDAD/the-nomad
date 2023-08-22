/*
      ___   _______     ____  __
     / _ \ |___ /\ \   / /  \/  |
    | | | |  |_ \ \ \ / /| |\/| |
    | |_| |____) | \ V / | |  | |
     \__\_______/   \_/  |_|  |_|


   Quake III Arena Virtual Machine
*/

/******************************************************************************
 * SYSTEM INCLUDE FILES
 ******************************************************************************/


/******************************************************************************
 * PROJECT INCLUDE FILES
 ******************************************************************************/

#include "../engine/n_shared.h"
#include "vm.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

//#define NO_VM_COMPILED 1
#define OPSTACK_SIZE 1024
#define MAX_VMSYSCALL_ARGS 16
#define MAX_VMMAIN_ARGS 13

#ifdef __GNUC__
#ifndef DEBUG_VM           /* can't use computed gotos in debug mode */
#define USE_COMPUTED_GOTOS /**< use computed gotos instead of a switch */
#endif
#endif

#define PROGRAM_STACK_EXTRA (32*1024)
#define VM_DATA_GUARD_SIZE 1024
#define OPCODE_TABLE_SIZE 64
#define OPCODE_TABLE_MASK (OPCODE_TABLE_SIZE - 1)
#define PROC_OPSTACK_SIZE 30
#define PROGRAM_STACK_SIZE VM_PROGRAM_STACK_SIZE

#define VM_MAX_BSS_LENGTH 10485760

const char *vmNames[MAX_VM_COUNT] = {
	"ui",
	"sgame",
	"script"
};

vm_t vmTable[MAX_VM_COUNT];
cvar_t *vm_rtChecks;
static int vm_debugLevel; /**< 0: be quiet, 1: debug msgs, 2: print op codes */


opcode_info_t ops[ OP_MAX ] =
{
	// size, stack, nargs, flags
	{ 0, 0, 0, 0 }, // undef
	{ 0, 0, 0, 0 }, // ignore
	{ 0, 0, 0, 0 }, // break

	{ 4, 0, 0, 0 }, // enter
	{ 4,-4, 0, 0 }, // leave
	{ 0, 0, 1, 0 }, // call
	{ 0, 4, 0, 0 }, // push
	{ 0,-4, 1, 0 }, // pop

	{ 4, 4, 0, 0 }, // const
	{ 4, 4, 0, 0 }, // local
	{ 0,-4, 1, 0 }, // jump

	{ 4,-8, 2, JUMP }, // eq
	{ 4,-8, 2, JUMP }, // ne

	{ 4,-8, 2, JUMP }, // lti
	{ 4,-8, 2, JUMP }, // lei
	{ 4,-8, 2, JUMP }, // gti
	{ 4,-8, 2, JUMP }, // gei

	{ 4,-8, 2, JUMP }, // ltu
	{ 4,-8, 2, JUMP }, // leu
	{ 4,-8, 2, JUMP }, // gtu
	{ 4,-8, 2, JUMP }, // geu

	{ 4,-8, 2, JUMP|FPU }, // eqf
	{ 4,-8, 2, JUMP|FPU }, // nef

	{ 4,-8, 2, JUMP|FPU }, // ltf
	{ 4,-8, 2, JUMP|FPU }, // lef
	{ 4,-8, 2, JUMP|FPU }, // gtf
	{ 4,-8, 2, JUMP|FPU }, // gef

	{ 0, 0, 1, 0 }, // load1
	{ 0, 0, 1, 0 }, // load2
	{ 0, 0, 1, 0 }, // load4
	{ 0,-8, 2, 0 }, // store1
	{ 0,-8, 2, 0 }, // store2
	{ 0,-8, 2, 0 }, // store4
	{ 1,-4, 1, 0 }, // arg
	{ 4,-8, 2, 0 }, // bcopy

	{ 0, 0, 1, 0 }, // sex8
	{ 0, 0, 1, 0 }, // sex16

	{ 0, 0, 1, 0 }, // negi
	{ 0,-4, 3, 0 }, // add
	{ 0,-4, 3, 0 }, // sub
	{ 0,-4, 3, 0 }, // divi
	{ 0,-4, 3, 0 }, // divu
	{ 0,-4, 3, 0 }, // modi
	{ 0,-4, 3, 0 }, // modu
	{ 0,-4, 3, 0 }, // muli
	{ 0,-4, 3, 0 }, // mulu

	{ 0,-4, 3, 0 }, // band
	{ 0,-4, 3, 0 }, // bor
	{ 0,-4, 3, 0 }, // bxor
	{ 0, 0, 1, 0 }, // bcom

	{ 0,-4, 3, 0 }, // lsh
	{ 0,-4, 3, 0 }, // rshi
	{ 0,-4, 3, 0 }, // rshu

	{ 0, 0, 1, FPU }, // negf
	{ 0,-4, 3, FPU }, // addf
	{ 0,-4, 3, FPU }, // subf
	{ 0,-4, 3, FPU }, // divf
	{ 0,-4, 3, FPU }, // mulf

	{ 0, 0, 1, 0 },   // cvif
	{ 0, 0, 1, FPU }  // cvfi
};


const char *opnames[ 256 ] = {
	"OP_UNDEF",

	"OP_IGNORE",

	"OP_BREAK",

	"OP_ENTER",
	"OP_LEAVE",
	"OP_CALL",
	"OP_PUSH",
	"OP_POP",

	"OP_CONST",

	"OP_LOCAL",

	"OP_JUMP",

	//-------------------

	"OP_EQ",
	"OP_NE",

	"OP_LTI",
	"OP_LEI",
	"OP_GTI",
	"OP_GEI",

	"OP_LTU",
	"OP_LEU",
	"OP_GTU",
	"OP_GEU",

	"OP_EQF",
	"OP_NEF",

	"OP_LTF",
	"OP_LEF",
	"OP_GTF",
	"OP_GEF",

	//-------------------

	"OP_LOAD1",
	"OP_LOAD2",
	"OP_LOAD4",
	"OP_STORE1",
	"OP_STORE2",
	"OP_STORE4",
	"OP_ARG",

	"OP_BLOCK_COPY",

	//-------------------

	"OP_SEX8",
	"OP_SEX16",

	"OP_NEGI",
	"OP_ADD",
	"OP_SUB",
	"OP_DIVI",
	"OP_DIVU",
	"OP_MODI",
	"OP_MODU",
	"OP_MULI",
	"OP_MULU",

	"OP_BAND",
	"OP_BOR",
	"OP_BXOR",
	"OP_BCOM",

	"OP_LSH",
	"OP_RSHI",
	"OP_RSHU",

	"OP_NEGF",
	"OP_ADDF",
	"OP_SUBF",
	"OP_DIVF",
	"OP_MULF",

	"OP_CVIF",
	"OP_CVFI"
};

static vmHeader_t* VM_LoadQVM(vm_t* vm, qboolean alloc);
static void VM_BlockCopy(unsigned int dest, unsigned int src, size_t n, vm_t* vm);

static char* VM_Indent(vm_t* vm);
static vmSymbol_t* VM_ValueToFunctionSymbol(vm_t* vm, int value);
static const char* VM_ValueToSymbol(vm_t* vm, int value);
static void VM_LoadSymbols(vm_t* vm);
static uint8_t* loadImage(const char* filepath, int* imageSize);
static void VM_StackTrace(vm_t* vm, int programCounter, int programStack);

void VM_Error(vm_t *vm, const char *fmt, ...)
{
    va_list argptr;
    char msg[MAX_MSG_SIZE];

    va_start(argptr, fmt);
    stbsp_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    Con_Error(false, "%s", msg);
    VM_Restart(vm);
}



/*
===============
VM_LoadSymbols
===============
*/
static void VM_LoadSymbols( vm_t *vm )
{
	union {
		char	*c;
		void	*v;
	} mapfile;
	const char *text_p, *token;
	char	name[MAX_GDR_PATH];
	char	symbols[MAX_GDR_PATH];
	vmSymbol_t	**prev, *sym;
	int		count;
	int		value;
	int		chars;
	int		segment;
	int		numInstructions;

	// don't load symbols if not developer
	if ( !Cvar_VariableInteger("c_devmode") ) {
		return;
	}

	COM_StripExtension(vm->name, name, sizeof(name));
	snprintf( symbols, sizeof( symbols ), "%s.map", name );
	FS_LoadFile( symbols, &mapfile.v );
	if ( !mapfile.c ) {
		Con_Printf( "Couldn't load symbol file: %s", symbols );
		return;
	}

	numInstructions = vm->instructionCount;

	// parse the symbols
	text_p = mapfile.c;
	prev = &vm->symbols;
	count = 0;

	while ( 1 ) {
		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			break;
		}
		segment = ParseHex( token );
		if ( segment ) {
			COM_Parse( &text_p );
			COM_Parse( &text_p );
			continue;		// only load code segment values
		}

		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			Con_Printf( "WARNING: incomplete line at end of file" );
			break;
		}
		value = ParseHex( token );

		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			Con_Printf( "WARNING: incomplete line at end of file" );
			break;
		}
		chars = strlen( token );
		sym = (vmSymbol_t *)Hunk_Alloc( sizeof( *sym ) + chars, "vmSymbol", h_high );
		*prev = sym;
		prev = &sym->next;
		sym->next = NULL;

		// convert value from an instruction number to a code offset
		if ( vm->instructionPointers && value >= 0 && value < numInstructions ) {
			value = vm->instructionPointers[value];
		}

		sym->symValue = value;
		N_strncpyz( sym->symName, token, chars + 1 );

		count++;
	}

	vm->numSymbols = count;
	Con_Printf( "%i symbols parsed from %s", count, symbols );
	FS_FreeFile( mapfile.v );
}



vm_t *VM_Restart(vm_t *vm)
{
    vmHeader_t *header;

    // DLL's can't be restarted in place
    if (vm->dllHandle) {
        syscall_t systemCall;
        dllSyscall_t dllSyscall;
        vmIndex_t index;

        index = vm->index;
        systemCall = vm->systemCall;
        dllSyscall = vm->dllSyscall;

        VM_Free(vm);

        vm = VM_Create(index, systemCall, dllSyscall, VMI_NATIVE);
        return vm;
    }

    // load the image
    if ((header = VM_LoadQVM(vm, qfalse)) == NULL) {
        Con_Printf(ERROR, "VM_Restart() failed");
        return vm;
    }

    Con_Printf("VM_Restart: restarting vm %s", vm->name);

    // free the original file
    FS_FreeFile(header);

    return vm;
}

/*
VM_LoadDLL: used to load a development dll instead of a virtual machine
*/
static void *GDR_DECL VM_LoadDLL(const char *name, vmMainFunc_t *entryPoint, dllSyscall_t systemcalls)
{
    const char *gamedir = Cvar_VariableString("fs_game");
    char filename[MAX_GDR_PATH];
    void *libHandle;
    dllEntry_t dllEntry;

    if (!*gamedir) {
        gamedir = Cvar_VariableString("fs_basegame");
    }

    libHandle = Sys_LoadDLL(name);
    if (!libHandle) {
        Con_Printf("VM_LoadDLL '%s' failed", name);
        return NULL;
    }

    Con_Printf("VM_LoadDLL '%s' ok", name);

    dllEntry = (dllEntry_t)Sys_GetProcAddress(libHandle, "dllEntry");
    *entryPoint = (vmMainFunc_t)Sys_GetProcAddress(libHandle, "vmMain");
    if (!*entryPoint || !dllEntry) {
        Sys_CloseDLL(libHandle);
        return NULL;
    }

    Con_Printf("VM_LoadDLL(%s) found **vmMain** at %p", name, (void *)*entryPoint);
    dllEntry(systemcalls);
    Con_Printf("VM_LoadDLL(%s) succeeded!", name);

    return libHandle;
}

vm_t *VM_Create(vmIndex_t index, syscall_t systemCalls, dllSyscall_t dllSyscalls, vmInterpret_t interpret)
{
    uint64_t remaining;
    const char *name;
    vmHeader_t *header;
    vm_t *vm;

    if (!systemCalls) {
        N_Error("VM_Create: bad parms");
    }

    if (index >= MAX_VM_COUNT) {
        N_Error("VM_Create: bad vm index %i", index);
    }
    remaining = Hunk_MemoryRemaining();

    vm = &vmTable[index];
    
    // see if we already have the vm
    if (vm->name) {
        if (vm->index != index) {
            Con_Printf(ERROR, "VM_Create: bad allocated vm index %i", vm->index);
            return NULL;
        }
        return vm;
    }

    name = vmNames[index];

    vm->name = name;
    vm->systemCall = systemCalls;
    vm->dllSyscall = dllSyscalls;
    vm->privateFlag = CVAR_PRIVATE;

    // never allow dll loading with a demo
    if (interpret == VMI_NATIVE) {
        if (Cvar_VariableInteger("fs_restrict")) {
            interpret = VMI_COMPILED;
        }
    }

    if (interpret == VMI_NATIVE) {
        // try to load as a system dll
        Con_Printf("Loading dll file %s.", name);
        vm->dllHandle = VM_LoadDLL(name, &vm->entryPoint, dllSyscalls);
        if (vm->dllHandle) {
            vm->privateFlag = 0; // allow reading private cvars
            vm->dataAlloc = ~0U;
            vm->dataMask = ~0U;
            vm->dataBase = 0;
            return vm;
        }
        Con_Printf("Failed to load dll, looking for qvm.");
        interpret = VMI_COMPILED;
    }

    // load the image
    if ((header = VM_LoadQVM(vm, qtrue)) == NULL) {
        return NULL;
    }

    // allocate space for the jump targets, which will be filled in by the compile/prep functions
	vm->instructionCount = header->instructionCount;
//	vm->instructionPointers = Hunk_Alloc(vm->instructionCount * sizeof(*vm->instructionPointers), "instructions", h_high);
	vm->instructionPointers = NULL;

	// copy or compile the instructions
	vm->codeLength = header->codeLength;

	// the stack is implicitly at the end of the image
	vm->programStack = vm->dataMask + 1;
	vm->stackBottom = vm->programStack - PROGRAM_STACK_SIZE - PROGRAM_STACK_EXTRA;

	vm->compiled = qfalse;

#ifdef NO_VM_COMPILED
	if ( interpret >= VMI_COMPILED ) {
		Con_Printf( "Architecture doesn't have a bytecode compiler, using interpreter" );
		interpret = VMI_BYTECODE;
	}
#else
	if ( interpret >= VMI_COMPILED ) {
		if ( VM_Compile( vm, header ) ) {
			vm->compiled = qtrue;
		}
	}
#endif
	// VM_Compile may have reset vm->compiled if compilation failed
	if ( !vm->compiled ) {
		if ( !VM_PrepareInterpreter2( vm, header ) ) {
			FS_FreeFile( header );	// free the original file
			VM_Free( vm );
			return NULL;
		}
	}

	// free the original file
	FS_FreeFile( header );

	// load the map file
	VM_LoadSymbols( vm );

	Con_Printf( "%s loaded in %lu bytes on the hunk", vm->name, remaining - Hunk_MemoryRemaining() );

	return vm;
}

static void VM_SwapInt( void *data, int32_t length )
{
#ifndef GDR_LITTLE_ENDIAN
	int32_t *ptr;
	int32_t i;
	ptr = (int32_t *) data;
	length /= sizeof( int32_t );
	for ( i = 0; i < length; i++ ) {
		ptr[ i ] = LittleInt( ptr[ i ] );
	}
#endif
}

static int forced_unload;

/*
==============
VM_Free
==============
*/
void VM_Free( vm_t *vm )
{
	if ( !vm ) {
		return;
	}

	if ( vm->callLevel ) {
		if ( !forced_unload ) {
			N_Error( "VM_Free(%s) on running vm", vm->name );
			return;
		}
		else {
			Con_Printf( "forcefully unloading %s vm", vm->name );
		}
	}

	if ( vm->destroy )
		vm->destroy( vm );

	if ( vm->dllHandle )
		Sys_CloseDLL( vm->dllHandle );

#if 0	// now automatically freed by hunk
	if ( vm->codeBase.ptr ) {
		Z_Free( vm->codeBase.ptr );
	}
	if ( vm->dataBase ) {
		Z_Free( vm->dataBase );
	}
	if ( vm->instructionPointers ) {
		Z_Free( vm->instructionPointers );
	}
#endif
	memset( vm, 0, sizeof( *vm ) );
}

void VM_Clear(void)
{
	for (uint32_t i = 0; i < MAX_VM_COUNT; i++) {
		VM_Free(&vmTable[i]);
	}
}

void VM_Forced_Unload_Start(void)
{
	forced_unload = 1;
}

void VM_Forced_Unload_Done(void)
{
	forced_unload = 0;
}

static vmHeader_t* VM_LoadQVM(vm_t* vm, qboolean alloc)
{
    uint64_t length;
    uint32_t dataLength;
    uint32_t dataAlloc;
    int i;
    qboolean tryjts;
    char filename[MAX_GDR_PATH], *errMsg;
    vmHeader_t *header;

    // load the buffer
    snprintf(filename, sizeof(filename), "%s.qvm", vm->name);
    length = FS_LoadFile(filename, (void **)&header);
    Con_Printf("Loading vm file %s...", filename);
    if (!header) {
        Con_Printf("Failed.");
        VM_Free(vm);
        return NULL;
    }

    // will also swap the header

    tryjts = qfalse;

#if 0
    if (header->vmMagic == VM_MAGIC_VER2) {
        Con_Printf("...which has vmMagic VM_MAGIC");
    }
    else {
#endif
        tryjts = qtrue;
#if 0
    }
#endif

    vm->exactDataLength = header->dataLength + header->litLength + header->bssLength;
	dataLength = vm->exactDataLength + PROGRAM_STACK_EXTRA;
	if ( dataLength < PROGRAM_STACK_SIZE + PROGRAM_STACK_EXTRA ) {
		dataLength = PROGRAM_STACK_SIZE + PROGRAM_STACK_EXTRA;
	}
	vm->dataLength = dataLength;

	// round up to next power of 2 so all data operations can
	// be mask protected
	for ( i = 0 ; dataLength > ( 1 << i ) ; i++ )
		;
	dataLength = 1 << i;

	// reserve some space for effective LOCAL+LOAD* checks
	dataAlloc = dataLength + VM_DATA_GUARD_SIZE;

	if ( dataLength >= (1U<<31) || dataAlloc >= (1U<<31) ) {
		// dataLenth is negative int32
		VM_Free( vm );
		FS_FreeFile( header );
		Con_Printf( ERROR, "%s: data segment is too large", __func__ );
		return NULL;
	}

	if ( alloc ) {
		// allocate zero filled space for initialized and uninitialized data
		vm->dataBase = (byte *)Hunk_Alloc( dataAlloc, "VMdataBase", h_high );
		vm->dataMask = dataLength - 1;
		vm->dataAlloc = dataAlloc;
	}
    else {
		// clear the data, but make sure we're not clearing more than allocated
		if ( vm->dataAlloc != dataAlloc ) {
			VM_Free( vm );
			FS_FreeFile( header );
			Con_Printf( WARNING, "Data region size of %s not matching after "
					"VM_Restart()", filename );
			return NULL;
		}
		memset( vm->dataBase, 0, vm->dataAlloc );
	}

	// copy the intialized data
	memcpy( vm->dataBase, (byte *)header + header->dataOffset, header->dataLength + header->litLength );

	// byte swap the longs
	VM_SwapInt( vm->dataBase, header->dataLength );

#if 0
	if ( header->vmMagic == VM_MAGIC_VER2 ) {
		int32_t previousNumJumpTableTargets = vm->numJumpTableTargets;

		header->jtrgLength &= ~0x03;

		vm->numJumpTableTargets = header->jtrgLength >> 2;
		Con_Printf( "Loading %i jump table targets", vm->numJumpTableTargets );

		if ( alloc ) {
			vm->jumpTableTargets = (int32_t *) Hunk_Alloc( header->jtrgLength, "jtrgLength", h_high );
		}
        else {
			if ( vm->numJumpTableTargets != previousNumJumpTableTargets ) {
				VM_Free( vm );
				FS_FreeFile( header );

				Con_Printf( WARNING, "Jump table size of %s not matching after "
					"VM_Restart()", filename );
				return NULL;
			}

			memset( vm->jumpTableTargets, 0, header->jtrgLength );
		}

		memcpy( vm->jumpTableTargets, (byte *)header + header->dataOffset +
				header->dataLength + header->litLength, header->jtrgLength );

		// byte swap the longs
		VM_SwapLongs( vm->jumpTableTargets, header->jtrgLength );
	}
#endif

#if 0
	if ( tryjts == qtrue && (length = Load_JTS( vm, crc32sum, NULL, vmPakIndex )) >= 0 ) {
		// we are trying to load newer file?
		if ( vm->jumpTableTargets && vm->numJumpTableTargets != length >> 2 ) {
			Con_Printf( WARNING, "Reload jts file" );
			vm->jumpTableTargets = NULL;
			alloc = qtrue;
		}
		vm->numJumpTableTargets = length >> 2;
		Con_Printf( "Loading %i external jump table targets", vm->numJumpTableTargets );
		if ( alloc == qtrue ) {
			vm->jumpTableTargets = (int32_t *) Hunk_Alloc( length, "jumpTables", h_high );
		} else {
		    memset( vm->jumpTableTargets, 0, length );
		}
		Load_JTS( vm, crc32sum, vm->jumpTableTargets, vmPakIndex );
	}
#endif
	vm->compiled = qfalse;

	return header;
}

/*
==============
VM_Call


Upon a system call, the stack will look like:

sp+32	parm1
sp+28	parm0
sp+24	return value
sp+20	return address
sp+16	local1
sp+14	local0
sp+12	arg1
sp+8	arg0
sp+4	return stack
sp		return address

An interpreted function will immediately execute
an OP_ENTER instruction, which will subtract space for
locals from sp
==============
*/

intptr_t GDR_DECL VM_Call(vm_t *vm, uint32_t numArgs, int32_t command, ...)
{
    intptr_t r;
    int i;

    if (!vm) {
        N_Error("VM_Call with NULL vm");
    }
#ifdef _NOMAD_DEBUG
    if (numArgs >= MAX_VMMAIN_ARGS) {
        Con_Printf(ERROR, "VM_Call: numArgs >= MAX_VMMAIN_ARGS");
        return -1;
    }
#endif

    ++vm->callLevel;

    // if we have a dll loaded, call it directly
    if (vm->entryPoint) {
        int32_t args[MAX_VMMAIN_ARGS - 1];
        va_list argptr;
        
        va_start(argptr, command);
        for (i = 0; i < numArgs; i++) {
            args[i] = va_arg(argptr, int32_t);
        }
        va_end(argptr);

        // add more agurments if you've changed MAX_VMMAIN_ARGS:
        r = vm->entryPoint(command, args[0], args[1], args[2]);
    }
    else {
#if defined(GDRi386) && !defined(__clang__)
#ifndef NO_VM_COMPILED
        if (vm->compiled)
            r = VM_CallCompiled(vm, numArgs + 1, &command);
		else
#endif
			r = VM_CallInterpreted2(vm, numArgs + 1, &command);
#else

		int32_t args[MAX_VMMAIN_ARGS];
        va_list argptr;
		args[0] = command;
        
        va_start(argptr, command);
        for (i = 0; i < numArgs; i++) {
            args[i] = va_arg(argptr, int32_t);
        }
        va_end(argptr);

#ifndef NO_VM_COMPILED
		if (vm->compiled)
			r = VM_CallCompiled(vm, numArgs + 1, args);
		else
#endif
			r = VM_CallInterpreted2(vm, numArgs + 1, args);
#endif
    }
	--vm->callLevel;

	return r;
}

int VM_MemoryRangeValid(intptr_t vmAddr, size_t len, vm_t* vm)
{
    if (!vmAddr || !vm)
        return -1;
    
    const unsigned dest     = vmAddr;
    const unsigned dataMask = vm->dataMask;
    if ((dest & dataMask) != dest || ((dest + len) & dataMask) != dest + len) {
        VM_Error(vm, "Memory access out of range");
        return -1;
    }
    else {
        return 0;
    }
}

static void VM_BlockCopy(unsigned int dest, unsigned int src, size_t n,
                         vm_t* vm)
{
    unsigned int dataMask = vm->dataMask;

    if ((dest & dataMask) != dest || (src & dataMask) != src ||
        ((dest + n) & dataMask) != dest + n ||
        ((src + n) & dataMask) != src + n) {
        VM_Error(vm,
                  "OP_BLOCK_COPY out of range");
        return;
    }

    memcpy(vm->dataBase + dest, vm->dataBase + src, n);
}

static void VM_IgnoreInstructions( instruction_t *buf, const int count )
{
	int i;

	for ( i = 0; i < count; i++ ) {
		memset( buf + i, 0, sizeof( *buf ) );
		buf[i].op = OP_IGNORE;
	}

	buf[0].value = count > 0 ? count - 1 : 0;
}


static int InvertCondition( int op )
{
	switch ( op ) {
		case OP_EQ: return OP_NE;   // == -> !=
		case OP_NE: return OP_EQ;   // != -> ==

		case OP_LTI: return OP_GEI;	// <  -> >=
		case OP_LEI: return OP_GTI;	// <= -> >
		case OP_GTI: return OP_LEI; // >  -> <=
		case OP_GEI: return OP_LTI; // >= -> <

		case OP_LTU: return OP_GEU;
		case OP_LEU: return OP_GTU;
		case OP_GTU: return OP_LEU;
		case OP_GEU: return OP_LTU;

		case OP_EQF: return OP_NEF;
		case OP_NEF: return OP_EQF;

		case OP_LTF: return OP_GEF;
		case OP_LEF: return OP_GTF;
		case OP_GTF: return OP_LEF;
		case OP_GEF: return OP_LTF;

		default: 
			Con_Printf( ERROR, "incorrect condition opcode %i", op );
			return op;
	}
}


/*
=================
VM_FindLocal

search for specified local variable until end of function
=================
*/
static qboolean VM_FindLocal( int32_t addr, const instruction_t *buf, const instruction_t *end, int32_t *back_addr )
{
	int32_t curr_addr = *back_addr;
	while ( buf < end ) {
		if ( buf->op == OP_LOCAL ) {
			if ( buf->value == addr ) {
				return qtrue;
			}
			++buf; continue;
		}
		if ( ops[ buf->op ].flags & JUMP ) {
			if ( buf->value < curr_addr ) {
				curr_addr = buf->value;
			}
			++buf; continue;
		}
		if ( buf->op == OP_JUMP ) {
			if ( buf->value && buf->value < curr_addr ) {
				curr_addr = buf->value;
			}
			++buf; continue;
		}
		if ( buf->op == OP_PUSH && (buf+1)->op == OP_LEAVE ) {
			break;
		}
		++buf;
	}
	*back_addr = curr_addr;
	return qfalse;
}


/*
=================
VM_Fixup

Do some corrections to fix known Q3LCC flaws
=================
*/
static void VM_Fixup( instruction_t *buf, int instructionCount )
{
	int n;
	instruction_t *i;

	i = buf;
	n = 0;

	while ( n < instructionCount ) {
		if ( i->op == OP_LOCAL ) {

			// skip useless sequences
			if ( (i+1)->op == OP_LOCAL && (i+0)->value == (i+1)->value && (i+2)->op == OP_LOAD4 && (i+3)->op == OP_STORE4 ) {
				VM_IgnoreInstructions( i, 4 );
				i += 4; n += 4;
				continue;
			}

			// [0]OP_LOCAL + [1]OP_CONST + [2]OP_CALL + [3]OP_STORE4
			if ( (i+1)->op == OP_CONST && (i+2)->op == OP_CALL && (i+3)->op == OP_STORE4 && !(i+4)->jused ) {
				// [4]OP_CONST|OP_LOCAL (dest) + [5]OP_LOCAL(temp) + [6]OP_LOAD4 + [7]OP_STORE4
				if ( (i+4)->op == OP_CONST || (i+4)->op == OP_LOCAL ) {
					if ( (i+5)->op == OP_LOCAL && (i+5)->value == (i+0)->value && (i+6)->op == OP_LOAD4 && (i+7)->op == OP_STORE4 ) {
						int32_t back_addr = n;
						int32_t curr_addr = n;
						qboolean do_break = qfalse;

						// make sure that address of (potentially) temporary variable is not referenced further in this function
						if ( VM_FindLocal( i->value, i + 8, buf + instructionCount, &back_addr ) ) {
							i++; n++;
							continue;
						}

						// we have backward jumps in code then check for references before current position
						while ( back_addr < curr_addr ) {
							curr_addr = back_addr;
							if ( VM_FindLocal( i->value, buf + back_addr, i, &back_addr ) ) {
								do_break = qtrue;
								break;
							}
						}
						if ( do_break ) {
							i++; n++;
							continue;
						}

						(i+0)->op = (i+4)->op;
						(i+0)->value = (i+4)->value;
						VM_IgnoreInstructions( i + 4, 4 );
						i += 8;
						n += 8;
						continue;
					}
				}
			}
		}

		if ( i->op == OP_LEAVE && !i->endp ) {
			if ( !(i+1)->jused && (i+1)->op == OP_CONST && (i+2)->op == OP_JUMP ) {
				int v = (i+1)->value;
				if ( buf[ v ].op == OP_PUSH && buf[ v+1 ].op == OP_LEAVE && buf[ v+1 ].endp ) {
					VM_IgnoreInstructions( i + 1, 2 );
					i += 3;
					n += 3;
					continue;
				}
			}
		}

		//n + 0: if ( cond ) goto label1;
		//n + 2: goto label2;
		//n + 3: label1:
		// ...
		//n + x: label2:
		if ( ( ops[i->op].flags & (JUMP | FPU) ) == JUMP && !(i+1)->jused && (i+1)->op == OP_CONST && (i+2)->op == OP_JUMP ) {
			if ( i->value == n + 3 && (i+1)->value >= n + 3 ) {
				i->op = InvertCondition( i->op );
				i->value = ( i + 1 )->value;
				VM_IgnoreInstructions( i + 1, 2 );
				i += 3;
				n += 3;
				continue;
			}
		}
		i++;
		n++;
	}
}


const char *VM_LoadInstructions( const byte *code_pos, uint32_t codeLength, uint32_t instructionCount, instruction_t *buf )
{
    static char errBuf[ 128 ];
	const byte *code_start, *code_end;
	int i, n, op0, op1, opStack;
	instruction_t *ci;

	code_start = code_pos; // for printing
	code_end = code_pos + codeLength;

	ci = buf;
	opStack = 0;
	op1 = OP_UNDEF;

	// load instructions and perform some initial calculations/checks
	for ( i = 0; i < instructionCount; i++, ci++, op1 = op0 ) {
		op0 = *code_pos;
		if ( op0 < 0 || op0 >= OP_MAX ) {
			sprintf( errBuf, "bad opcode %02X at offset %d", op0, (int)(code_pos - code_start) );
			return errBuf;
		}
		n = ops[ op0 ].size;
		if ( code_pos + 1 + n  > code_end ) {
			sprintf( errBuf, "code_pos > code_end" );
			return errBuf;
		}
		code_pos++;
		ci->op = op0;
		if ( n == 4 ) {
			ci->value = LittleInt( *((int32_t*)code_pos) );
			code_pos += 4;
		} else if ( n == 1 ) {
			ci->value = *((unsigned char*)code_pos);
			code_pos += 1;
		} else {
			ci->value = 0;
		}

		if ( ops[ op0 ].flags & FPU ) {
			ci->fpu = 1;
		}

		// setup jump value from previous const
		if ( op0 == OP_JUMP && op1 == OP_CONST ) {
			ci->value = (ci-1)->value;
		}

		ci->opStack = opStack;
		opStack += ops[ op0 ].stack;
	}

	return NULL;
}

static qboolean safe_address( instruction_t *ci, instruction_t *proc, int dataLength )
{
	if ( ci->op == OP_LOCAL ) {
		// local address can't exceed programStack frame plus 256 bytes of passed arguments
		if ( ci->value < 8 || ( proc && ci->value >= proc->value + 256 ) )
			return qfalse;
		return qtrue;
	}

	if ( ci->op == OP_CONST ) {
		// constant address can't exceed data segment
		if ( ci->value >= dataLength || ci->value < 0 )
			return qfalse;
		return qtrue;
	}

	return qfalse;
}

/*
===============================
VM_CheckInstructions

performs additional consistency and security checks
===============================
*/
const char *VM_CheckInstructions( instruction_t *buf, uint32_t instructionCount, const int32_t *jumpTableTargets, uint32_t numJumpTableTargets, uint32_t dataLength )
{
	static char errBuf[ 128 ];
	instruction_t *opStackPtr[ PROC_OPSTACK_SIZE ];
	int i, m, n, v, op0, op1, opStack, pstack;
	instruction_t *ci, *proc;
	int startp, endp;
	int safe_stores;
	int unsafe_stores;

	ci = buf;
	opStack = 0;

	// opstack checks
	for ( i = 0; i < instructionCount; i++, ci++ ) {
		opStack += ops[ ci->op ].stack;
		if ( opStack < 0 ) {
			sprintf( errBuf, "opStack underflow at %i", i );
			return errBuf;
		}
		if ( opStack >= PROC_OPSTACK_SIZE * 4 ) {
			sprintf( errBuf, "opStack overflow at %i", i );
			return errBuf;
		}
	}

	ci = buf;
	pstack = 0;
	opStack = 0;
	safe_stores = 0;
	unsafe_stores = 0;
	op1 = OP_UNDEF;
	proc = NULL;
	memset( opStackPtr, 0, sizeof( opStackPtr ) );

	startp = 0;
	endp = instructionCount - 1;

	// Additional security checks

	for ( i = 0; i < instructionCount; i++, ci++, op1 = op0 ) {
		op0 = ci->op;

		m = ops[ ci->op ].stack;
		opStack += m;
		if ( m >= 0 ) {
			// do some FPU type promotion for more efficient loads
			if ( ci->fpu && ci->op != OP_CVIF ) {
				opStackPtr[ opStack / 4 ]->fpu = 1;
			}
			opStackPtr[ opStack >> 2 ] = ci;
		} else {
			if ( ci->fpu ) {
				if ( m <= -8 ) {
					opStackPtr[ opStack / 4 + 1 ]->fpu = 1;
					opStackPtr[ opStack / 4 + 2 ]->fpu = 1;
				} else {
					opStackPtr[ opStack / 4 + 0 ]->fpu = 1;
					opStackPtr[ opStack / 4 + 1 ]->fpu = 1;
				}
			} else {
				if ( m <= -8 ) {
					//
				} else {
					opStackPtr[ opStack / 4 + 0 ] = ci;
				}
			}
		}

		// function entry
		if ( op0 == OP_ENTER ) {
			// missing block end
			if ( proc || ( pstack && op1 != OP_LEAVE ) ) {
				sprintf( errBuf, "missing proc end before %i", i );
				return errBuf;
			}
			if ( ci->opStack != 0 ) {
				v = ci->opStack;
				sprintf( errBuf, "bad entry opstack %i at %i", v, i );
				return errBuf;
			}
			v = ci->value;
			if ( v < 0 || v >= PROGRAM_STACK_SIZE || (v & 3) ) {
				sprintf( errBuf, "bad entry programStack %i at %i", v, i );
				return errBuf;
			}

			pstack = ci->value;

			// mark jump target
			ci->jused = 1;
			proc = ci;
			startp = i + 1;

			// locate endproc
			for ( endp = 0, n = i+1 ; n < instructionCount; n++ ) {
				if ( buf[n].op == OP_PUSH && buf[n+1].op == OP_LEAVE ) {
					buf[n+1].endp = 1;
					endp = n;
					break;
				}
			}

			if ( endp == 0 ) {
				sprintf( errBuf, "missing end proc for %i", i );
				return errBuf;
			}

			continue;
		}

		// proc opstack will carry max.possible opstack value
		if ( proc && ci->opStack > proc->opStack )
			proc->opStack = ci->opStack;

		// function return
		if ( op0 == OP_LEAVE ) {
			// bad return programStack
			if ( pstack != ci->value ) {
				v = ci->value;
				sprintf( errBuf, "bad programStack %i at %i", v, i );
				return errBuf;
			}
			// bad opStack before return
			if ( ci->opStack != 4 ) {
				v = ci->opStack;
				sprintf( errBuf, "bad opStack %i at %i", v, i );
				return errBuf;
			}
			v = ci->value;
			if ( v < 0 || v >= PROGRAM_STACK_SIZE || (v & 3) ) {
				sprintf( errBuf, "bad return programStack %i at %i", v, i );
				return errBuf;
			}
			if ( op1 == OP_PUSH ) {
				if ( proc == NULL ) {
					sprintf( errBuf, "unexpected proc end at %i", i );
					return errBuf;
				}
				proc = NULL;
				startp = i + 1; // next instruction
				endp = instructionCount - 1; // end of the image
			}
			continue;
		}

		// conditional jumps
		if ( ops[ ci->op ].flags & JUMP ) {
			v = ci->value;
			// conditional jumps should have opStack >= 8
			if ( ci->opStack < 8 ) {
				sprintf( errBuf, "bad jump opStack %i at %i", ci->opStack, i );
				return errBuf;
			}
			//if ( v >= header->instructionCount ) {
			// allow only local proc jumps
			if ( v < startp || v > endp ) {
				sprintf( errBuf, "jump target %i at %i is out of range (%i,%i)", v, i-1, startp, endp );
				return errBuf;
			}
			if ( buf[v].opStack != ci->opStack - 8 ) {
				n = buf[v].opStack;
				sprintf( errBuf, "jump target %i has bad opStack %i", v, n );
				return errBuf;
			}
			// mark jump target
			buf[v].jused = 1;
			continue;
		}

		// unconditional jumps
		if ( op0 == OP_JUMP ) {
			// jumps should have opStack >= 4
			if ( ci->opStack < 4 ) {
				sprintf( errBuf, "bad jump opStack %i at %i", ci->opStack, i );
				return errBuf;
			}
			if ( op1 == OP_CONST ) {
				v = buf[i-1].value;
				// allow only local jumps
				if ( v < startp || v > endp ) {
					sprintf( errBuf, "jump target %i at %i is out of range (%i,%i)", v, i-1, startp, endp );
					return errBuf;
				}
				if ( buf[v].opStack != ci->opStack - 4 ) {
					n = buf[v].opStack;
					sprintf( errBuf, "jump target %i has bad opStack %i", v, n );
					return errBuf;
				}
				if ( buf[v].op == OP_ENTER ) {
					n = buf[v].op;
					sprintf( errBuf, "jump target %i has bad opcode %s", v, opnames[ n ] );
					return errBuf;
				}
				if ( v == (i-1) ) {
					sprintf( errBuf, "self loop at %i", v );
					return errBuf;
				}
				// mark jump target
				buf[v].jused = 1;
			} else {
				if ( proc )
					proc->swtch = 1;
				else
					ci->swtch = 1;
			}
			continue;
		}

		if ( op0 == OP_CALL ) {
			if ( ci->opStack < 4 ) {
				sprintf( errBuf, "bad call opStack at %i", i );
				return errBuf;
			}
			if ( op1 == OP_CONST ) {
				v = buf[i-1].value;
				// analyse only local function calls
				if ( v >= 0 ) {
					if ( v >= instructionCount ) {
						sprintf( errBuf, "call target %i is out of range", v );
						return errBuf;
					}
					if ( buf[v].op != OP_ENTER ) {
						n = buf[v].op;
						sprintf( errBuf, "call target %i has bad opcode %s", v, opnames[ n ] );
						return errBuf;
					}
					if ( v == 0 ) {
						sprintf( errBuf, "explicit vmMain call inside VM at %i", i );
						return errBuf;
					}
					// mark jump target
					buf[v].jused = 1;
				}
			}
			continue;
		}

		if ( ci->op == OP_ARG ) {
			v = ci->value & 255;
			if ( proc == NULL ) {
				sprintf( errBuf, "missing proc frame for %s %i at %i", opnames[ ci->op ], v, i );
				return errBuf;
			}
			// argument can't exceed programStack frame
			if ( v < 8 || v > pstack - 4 || (v & 3) ) {
				sprintf( errBuf, "bad argument address %i at %i", v, i );
				return errBuf;
			}
			continue;
		}

		if ( ci->op == OP_LOCAL ) {
			v = ci->value;
			if ( proc == NULL ) {
				sprintf( errBuf, "missing proc frame for %s %i at %i", opnames[ ci->op ], v, i );
				return errBuf;
			}
			if ( (ci+1)->op == OP_LOAD4 || (ci+1)->op == OP_LOAD2 || (ci+1)->op == OP_LOAD1 ) {
				if ( !safe_address( ci, proc, dataLength ) ) {
					sprintf( errBuf, "bad %s address %i at %i", opnames[ ci->op ], v, i );
					return errBuf;
				}
			}
			continue;
		}

		if ( ci->op == OP_LOAD4 && op1 == OP_CONST ) {
			v = (ci-1)->value;
			if ( v < 0 || v > dataLength - 4 ) {
				sprintf( errBuf, "bad %s address %i at %i", opnames[ ci->op ], v, i - 1 );
				return errBuf;
			}
			continue;
		}

		if ( ci->op == OP_LOAD2 && op1 == OP_CONST ) {
			v = (ci-1)->value;
			if ( v < 0 || v > dataLength - 2 ) {
				sprintf( errBuf, "bad %s address %i at %i", opnames[ ci->op ], v, i - 1 );
				return errBuf;
			}
			continue;
		}

		if ( ci->op == OP_LOAD1 && op1 == OP_CONST ) {
			v =  (ci-1)->value;
			if ( v < 0 || v > dataLength - 1 ) {
				sprintf( errBuf, "bad %s address %i at %i", opnames[ ci->op ], v, i - 1 );
				return errBuf;
			}
			continue;
		}

		if ( ci->op == OP_STORE4 || ci->op == OP_STORE2 || ci->op == OP_STORE1 ) {
			instruction_t *x = opStackPtr[ opStack / 4 + 1 ];
			if ( x->op == OP_CONST || x->op == OP_LOCAL ) {
				if ( safe_address( x, proc, dataLength ) ) {
					ci->safe = 1;
					safe_stores++;
					continue;
				} else {
					sprintf( errBuf, "bad %s address %i at %i", opnames[ ci->op ], x->value, (int)(x - buf) );
					return errBuf;
				}
			}
			unsafe_stores++;
			continue;
		}

		if ( ci->op == OP_BLOCK_COPY ) {
			instruction_t *src = opStackPtr[ opStack / 4 + 2 ];
			instruction_t *dst = opStackPtr[ opStack / 4 + 1 ];
			int safe = 0;
			v = ci->value;
			if ( v >= dataLength ) {
				sprintf( errBuf, "bad count %i for block copy at %i", v, i - 1 );
				return errBuf;
			}
			if ( src->op == OP_LOCAL || src->op == OP_CONST ) {
				if ( !safe_address( src, proc, dataLength ) ) {
					sprintf( errBuf, "bad src for block copy at %i", (int)(dst - buf) );
					return errBuf;
				}
				src->safe = 1;
				safe++;
			}
			if ( dst->op == OP_LOCAL || dst->op == OP_CONST ) {
				if ( !safe_address( dst, proc, dataLength ) ) {
					sprintf( errBuf, "bad dst for block copy at %i", (int)(dst - buf) );
					return errBuf;
				}
				dst->safe = 1;
				safe++;
			}
			if ( safe == 2 ) {
				ci->safe = 1;
			}
		}
//		op1 = op0;
//		ci++;
	}

	if ( ( safe_stores + unsafe_stores ) > 0 ) {
		Con_Printf( DEBUG, "%s: safe stores - %i (%i%%)", __func__, safe_stores, safe_stores * 100 / ( safe_stores + unsafe_stores ) );
	}

	if ( op1 != OP_UNDEF && op1 != OP_LEAVE ) {
		sprintf( errBuf, "missing return instruction at the end of the image" );
		return errBuf;
	}

	// ensure that the optimization pass knows about all the jump table targets
	if ( jumpTableTargets ) {
		// first pass - validate
		for( i = 0; i < numJumpTableTargets; i++ ) {
			n = jumpTableTargets[ i ];
			if ( n < 0 || n >= instructionCount ) {
				Con_Printf( WARNING, "jump target %i set on instruction %i that is out of range [0..%i]",
					i, n, instructionCount - 1 );
				break;
			}
			if ( buf[n].opStack != 0 ) {
				Con_Printf( WARNING, "jump target %i set on instruction %i (%s) with bad opStack %i",
					i, n, opnames[ buf[n].op ], buf[n].opStack );
				break;
			}
		}
		if ( i != numJumpTableTargets ) {
			// we may trap this on buggy VM_MAGIC_VER2 images
			// but we can safely optimize code even without JTRGSEG
			// so just switch to VM_MAGIC path here
			goto __noJTS;
		}
		// second pass - apply
		for( i = 0; i < numJumpTableTargets; i++ ) {
			n = jumpTableTargets[ i ];
			buf[ n ].jused = 1;
		}
	} else {
__noJTS:
		v = 0;
		// instructions with opStack > 0 can't be jump labels so it is safe to optimize/merge
		for ( i = 0, ci = buf; i < instructionCount; i++, ci++ ) {
			if ( ci->op == OP_ENTER ) {
				v = ci->swtch;
				continue;
			}
			// if there is a switch statement in function -
			// mark all potential jump labels
			if ( ci->swtch )
				v = ci->swtch;
			if ( ci->opStack > 0 )
				ci->jused = 0;
			else if ( v )
				ci->jused = 1;
		}
	}

	VM_Fixup( buf, instructionCount );

	return NULL;
}

void VM_StackTrace( vm_t *vm, int programCounter, int programStack )
{
	int		count;

	count = 0;
	do {
		Con_Printf( "%s", VM_ValueToSymbol( vm, programCounter ) );
		programStack =  *(int *)&vm->dataBase[programStack+4];
		programCounter = *(int *)&vm->dataBase[programStack];
	} while ( programCounter != -1 && ++count < 32 );

}

char *VM_Indent( vm_t *vm ) {
	static char	*string = "                                        ";
	if ( vm->callLevel > 20 ) {
		return string;
	}
	return string + 2 * ( 20 - vm->callLevel );
}


// macro opcode sequences
typedef enum {
	MOP_LOCAL_LOAD4 = OP_MAX,
	MOP_LOCAL_LOAD4_CONST,
	MOP_LOCAL_LOCAL,
	MOP_LOCAL_LOCAL_LOAD4,
} macro_op_t;


/*
=================
VM_FindMOps

Search for known macro-op sequences
=================
*/
static void VM_FindMOps( instruction_t *buf, uint32_t instructionCount )
{
	uint32_t i, op0;
	instruction_t *ci;

	ci = buf;
	i = 0;

	while ( i < instructionCount ) {
		op0 = ci->op;

		if ( op0 == OP_LOCAL ) {
			if ( (ci+1)->op == OP_LOAD4 && (ci+2)->op == OP_CONST ) {
				ci->op = MOP_LOCAL_LOAD4_CONST;
				ci += 3; i += 3;
				continue;
			}
			if ( (ci+1)->op == OP_LOAD4 ) {
				ci->op = MOP_LOCAL_LOAD4;
				ci += 2; i += 2;
				continue;
			}

			if ( (ci+1)->op == OP_LOCAL && (ci+2)->op == OP_LOAD4 ) {
				ci->op = MOP_LOCAL_LOCAL_LOAD4;
				ci += 3; i += 3;
				continue;
			}
			if ( (ci+1)->op == OP_LOCAL ) {
				ci->op = MOP_LOCAL_LOCAL;
				ci += 2; i += 2;
				continue;
			}
		}

		ci++;
		i++;
	}
}

qboolean VM_PrepareInterpreter2(vm_t* vm, vmHeader_t* header)
{
    const char *errMsg;
    instruction_t *buf;

    buf = (instruction_t *)Hunk_Alloc((vm->instructionCount + 8) * sizeof(instruction_t), "VMinstruct", h_low);

    errMsg = VM_LoadInstructions((byte *)header + header->codeOffset, header->codeLength, header->instructionCount, buf);
#if 0
    if (!errMsg) {
        errMsg = VM_CheckInstructions(buf, vm->instructionCount,  vm->jumpTableTargets, vm->numJumpTableTargets, vm->exactDataLength);
    }
#endif
    if (errMsg) {
        Con_Printf("VM_PrepareIntepreter2 error: %s", errMsg);
        return qfalse;
    }

    VM_FindMOps(buf, vm->instructionCount);

    vm->codeBase.ptr = (byte *)buf;
    return qtrue;
}

/* DEBUG FUNCTIONS */
/* --------------- */

void VM_Debug(int level)
{
    vm_debugLevel = level;
}

#ifdef DEBUG_VM
static const char* VM_ValueToSymbol(vm_t* vm, int value)
{
    vmSymbol_t* sym;
    static char text[MAX_TOKEN_CHARS];

    sym = vm->symbols;
    if (!sym) {
        return "NO SYMBOLS";
    }

    /* find the symbol */
    while (sym->next && sym->next->symValue <= value) {
        sym = sym->next;
    }

    if (value == sym->symValue)
    {
        return sym->symName;
    }

    snprintf(text, sizeof(text), "%s+%i", sym->symName, value - sym->symValue);

    return text;
}

static vmSymbol_t* VM_ValueToFunctionSymbol(vm_t* vm, int value)
{
    vmSymbol_t*       sym;
    static vmSymbol_t nullSym;

    sym = vm->symbols;
    if (!sym)
    {
        return &nullSym;
    }

    while (sym->next && sym->next->symValue <= value)
    {
        sym = sym->next;
    }

    return sym;
}

static int VM_ProfileSort(const void* a, const void* b)
{
    vmSymbol_t *sa, *sb;

    sa = *(vmSymbol_t**)a;
    sb = *(vmSymbol_t**)b;

    if (sa->profileCount < sb->profileCount)
        return -1;
    if (sa->profileCount > sb->profileCount)
        return 1;
    
    return 0;
}

void VM_VmProfile_f(const vm_t* vm)
{
    vmSymbol_t **sorted, *sym;
    int          i;
    float        total;

    if (!vm)
        return;

    if (vm->numSymbols < 1)
        return;

    sorted = (vmSymbol_t **)Z_Malloc(vm->numSymbols * sizeof(*sorted), TAG_STATIC, &sorted, "dbgSymbols");
    sorted[0] = vm->symbols;
    total     = (float)sorted[0]->profileCount;
    for (i = 1; i < vm->numSymbols; i++) {
        sorted[i] = sorted[i - 1]->next;
        total += sorted[i]->profileCount;
    }

    qsort(sorted, vm->numSymbols, sizeof(*sorted), VM_ProfileSort);

    for (i = 0; i < vm->numSymbols; i++) {
        int perc;

        sym = sorted[i];

        perc = (int)(100 * (float)sym->profileCount / total);
        Con_Printf("%2i%% %9i %s", perc, sym->profileCount, sym->symName);
        sym->profileCount = 0;
    }

    Con_Printf("    %9.0f total", total);

    Z_Free(sorted);
}
#else
void VM_VmProfile_f(const vm_t* vm)
{
    (void)vm;
}
#endif