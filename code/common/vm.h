#ifndef __Q3VM_H
#define __Q3VM_H

#ifdef __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#ifdef _NOMAD_DEBUG
#define DEBUG_VM
#endif

#define VM_MAGIC 0x12721444
#define VM_PROGRAM_STACK_SIZE 0x10000
#define VM_MAX_IMAGE_SIZE 0x400000
#define VM_MAX_QPATH 64

#define VMA(x) VM_ArgPtr(args[x])
GDR_INLINE float _vmf(intptr_t x)
{
	floatint_t v;
	v.i = (int32_t)x;
	return v.f;
}
#define VMF(x) _vmf(args[x])

#define OPSTACK_SIZE 1024
#define MAX_VMSYSCALL_ARGS 16
#define MAX_VMMAIN_ARGS 13

#ifdef __GNUC__
#ifndef DEBUG_VM           /* can't use computed gotos in debug mode */
#define USE_COMPUTED_GOTOS /**< use computed gotos instead of a switch */
#endif
#endif

#define OPCODE_TABLE_SIZE 64
#define OPCODE_TABLE_MASK (OPCODE_TABLE_SIZE - 1)

typedef enum {
    OP_UNDEF, /* Error: VM halt */

    OP_IGNORE, /* No operation */

    OP_BREAK, /* vm->breakCount++ */

    OP_ENTER, /* Begin subroutine. */
    OP_LEAVE, /* End subroutine. */
    OP_CALL,  /* Call subroutine. */
    OP_PUSH,  /* Push to stack. */
    OP_POP,   /* Discard top-of-stack. */

    OP_CONST, /* Load constant to stack. */
    OP_LOCAL, /* Get local variable. */

    OP_JUMP, /* Unconditional jump. */

    /*-------------------*/

    OP_EQ, /* Compare integers, jump if equal. */
    OP_NE, /* Compare integers, jump if not equal. */

    OP_LTI, /* Compare integers, jump if less-than. */
    OP_LEI, /* Compare integers, jump if less-than-or-equal. */
    OP_GTI, /* Compare integers, jump if greater-than. */
    OP_GEI, /* Compare integers, jump if greater-than-or-equal. */

    OP_LTU, /* Compare unsigned integers, jump if less-than */
    OP_LEU, /* Compare unsigned integers, jump if less-than-or-equal */
    OP_GTU, /* Compare unsigned integers, jump if greater-than */
    OP_GEU, /* Compare unsigned integers, jump if greater-than-or-equal */

    OP_EQF, /* Compare floats, jump if equal */
    OP_NEF, /* Compare floats, jump if not-equal */

    OP_LTF, /* Compare floats, jump if less-than */
    OP_LEF, /* Compare floats, jump if less-than-or-equal */
    OP_GTF, /* Compare floats, jump if greater-than */
    OP_GEF, /* Compare floats, jump if greater-than-or-equal */

    /*-------------------*/

    OP_LOAD1,  /* Load 1-byte from memory */
    OP_LOAD2,  /* Load 2-bytes from memory */
    OP_LOAD4,  /* Load 4-bytes from memory */
    OP_STORE1, /* Store 1-byte to memory */
    OP_STORE2, /* Store 2-byte to memory */
    OP_STORE4, /* *(stack[top-1]) = stack[top] */
    OP_ARG,    /* Marshal argument */

    OP_BLOCK_COPY, /* memcpy */

    /*-------------------*/

    OP_SEX8,  /* Sign-Extend 8-bit */
    OP_SEX16, /* Sign-Extend 16-bit */

    OP_NEGI, /* Negate integer. */
    OP_ADD,  /* Add integers (two's complement). */
    OP_SUB,  /* Subtract integers (two's complement). */
    OP_DIVI, /* Divide signed integers. */
    OP_DIVU, /* Divide unsigned integers. */
    OP_MODI, /* Modulus (signed). */
    OP_MODU, /* Modulus (unsigned). */
    OP_MULI, /* Multiply signed integers. */
    OP_MULU, /* Multiply unsigned integers. */

    OP_BAND, /* Bitwise AND */
    OP_BOR,  /* Bitwise OR */
    OP_BXOR, /* Bitwise eXclusive-OR */
    OP_BCOM, /* Bitwise COMplement */

    OP_LSH,  /* Left-shift */
    OP_RSHI, /* Right-shift (algebraic; preserve sign) */
    OP_RSHU, /* Right-shift (bitwise; ignore sign) */

    OP_NEGF, /* Negate float */
    OP_ADDF, /* Add floats */
    OP_SUBF, /* Subtract floats */
    OP_DIVF, /* Divide floats */
    OP_MULF, /* Multiply floats */

    OP_CVIF, /* Convert to integer from float */
    OP_CVFI, /* Convert to float from integer */

    OP_MAX /* Make this the last item */
} opcode_t;

typedef enum
{
    VM_NO_ERROR                    = 0,
    VM_INVALID_POINTER             = -1,
    VM_FAILED_TO_LOAD_BYTECODE     = -2,
    VM_NO_SYSCALL_CALLBACK         = -3,
    VM_FREE_ON_RUNNING_VM          = -4,
    VM_BLOCKCOPY_OUT_OF_RANGE      = -5,
    VM_PC_OUT_OF_RANGE             = -6,
    VM_JUMP_TO_INVALID_INSTRUCTION = -7,
    VM_STACK_OVERFLOW              = -8,
    VM_STACK_MISALIGNED            = -9,
    VM_OP_LOAD4_MISALIGNED         = -10,
    VM_STACK_ERROR                 = -11,
    VM_DATA_OUT_OF_RANGE           = -12,
    VM_MALLOC_FAILED               = -13,
    VM_BAD_INSTRUCTION             = -14,
    VM_NOT_LOADED                  = -15
} vmErrorCode_t;

typedef enum
{
    VM_ALLOC_CODE_SEC             = 0, /**< Bytecode code section */
    VM_ALLOC_DATA_SEC             = 1, /**< Bytecode data section */
    VM_ALLOC_INSTRUCTION_POINTERS = 2, /**< Bytecode instruction pointers */
    VM_ALLOC_DEBUG                = 3, /**< DEBUG_VM functions */
    VM_ALLOC_TYPE_MAX                  /**< Last item in vmMallocType_t */
} vmMallocType_t;

typedef struct {
    int32_t vmMagic;

	int32_t instructionCount;

	int32_t codeOffset;
	int32_t codeLength;

	int32_t dataOffset;
	int32_t dataLength;
	int32_t litLength;			// ( dataLength - litLength ) should be byteswapped on load
	int32_t bssLength;			// zero filled memory appended to datalength

	//!!! below here is VM_MAGIC_VER2 !!!
	int32_t jtrgLength;			// number of jump table targets
} vmHeader_t;

typedef struct vmSymbol_s
{
    struct vmSymbol_s* next; /**< Linked list of symbols */

    int32_t symValue; /**< Value of symbol that we want to have the ASCII name for
                     */
    int32_t  profileCount; /**< For the runtime profiler. +1 for each call. */
    char symName[1];   /**< Variable sized symbol name. Space is reserved by
                          malloc at load time. */
} vmSymbol_t;

typedef union vmFunc_u {
    byte *ptr;
    void (*func)(void);
} vmFunc_t;

typedef struct {
	int32_t	value;     // 32
	byte	op;        // 8
	byte	opStack;   // 8
	unsigned jused:1;  // this instruction is a jump target
	unsigned swtch:1;  // indirect jump
	unsigned safe:1;   // non-masked OP_STORE*
	unsigned endp:1;   // for last OP_LEAVE instruction
	unsigned fpu:1;    // load into FPU register
	unsigned njump:1;  // near jump
} instruction_t;

#ifdef __cplusplus
typedef intptr_t (GDR_DECL *vmMainFunc_t)(int command, int arg0, int arg1, int arg2);
typedef intptr_t (*syscall_t)(intptr_t *parms);
typedef intptr_t (GDR_DECL *dllSyscall_t)(intptr_t callNum, ...);
typedef void (GDR_DECL *dllEntry_t)(dllSyscall_t syscallptr);
#endif

typedef enum
{
    VM_UI = 0,
    VM_SGAME,
    VM_SCRIPT
} vmIndex_t;

// flags for vm_rtChecks cvar
#define VM_RTCHECK_PSTACK  1
#define VM_RTCHECK_OPSTACK 2
#define VM_RTCHECK_JUMP    4
#define VM_RTCHECK_DATA    8


typedef enum {
	VMI_NATIVE,
	VMI_BYTECODE,
	VMI_COMPILED
} vmInterpret_t;

typedef enum {
	TRAP_MEMSET = 101,
	TRAP_MEMCPY,
	TRAP_STRNCPY,
	TRAP_STRNCMP,
	TRAP_FLOOR,
	TRAP_ACOS,
	TRAP_SIN,
	TRAP_COS,
	TRAP_ATAN2,
	TRAP_CEIL,
	TRAP_TAN,
    TRAP_SQRT,
    TRAP_POW,
    TRAP_STRLEN,
    TRAP_ATOF,
    TRAP_STRSTR,
    TRAP_STRRCHR,
    TRAP_VSPRINTF,
} sharedTraps_t;

typedef struct vm_s
{
    syscall_t	systemCall;
	byte		*dataBase;
	int32_t		*opStack;			// pointer to local function stack
	int32_t		*opStackTop;

	int32_t		programStack;		// the vm may be recursively entered
	int32_t		stackBottom;		// if programStack < stackBottom, error

	//------------------------------------

	const char	*name;				// module should be bare: "cgame", not "cgame.dll" or "vm/cgame.qvm"
	vmIndex_t	index;

	// for dynamic linked modules
	void		*dllHandle;
	vmMainFunc_t entryPoint;
	dllSyscall_t dllSyscall;
	void (*destroy)(struct vm_s* self);

	// for interpreted modules
	//qboolean	currentlyInterpreting;

	qboolean	compiled;

	vmFunc_t	codeBase;
	uint32_t 	codeSize;			// code + jump targets, needed for proper munmap()
	uint32_t 	codeLength;			// just for information

	int32_t		instructionCount;
	intptr_t	*instructionPointers;

	uint32_t	dataMask;
	uint32_t	dataLength;			// data segment length
	uint32_t	exactDataLength;	// from qvm header
	uint32_t	dataAlloc;			// actually allocated, for mmap()/munmap()

	int32_t		numSymbols;
	vmSymbol_t	*symbols;

	int32_t		callLevel;			// counts recursive VM_Call
	int32_t		breakFunction;		// increment breakCount on function entry to this
	int32_t		breakCount;

	int32_t		*jumpTableTargets;
	int32_t		numJumpTableTargets;

	uint32_t	crc32sum;

	qboolean	forceDataMask;

	uint32_t	privateFlag;
} vm_t;

#define MAX_VM_COUNT 3

extern cvar_t *vm_rtChecks;

extern const char *vmNames[MAX_VM_COUNT];

void VM_Forced_Unload_Start(void);
void VM_Forced_Unload_Done(void);
void VM_Clear(void);

void VM_Error(vm_t *vm, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));
vm_t *VM_Restart(vm_t *vm);

qboolean VM_Compile( vm_t *vm, vmHeader_t *header );
int32_t VM_CallCompiled( vm_t *vm, uint32_t nargs, int32_t *args );

vm_t *VM_Create(vmIndex_t index, syscall_t systemCalls, dllSyscall_t dllSyscalls, vmInterpret_t interpret);
qboolean VM_PrepareInterpreter2(vm_t* vm, vmHeader_t* header);
int VM_CallInterpreted2(vm_t* vm, uint32_t nargs, int32_t* args);

vmSymbol_t *VM_ValueToFunctionSymbol( vm_t *vm, uint32_t value );
int VM_SymbolToValue( vm_t *vm, const char *symbol );
const char *VM_ValueToSymbol( vm_t *vm, uint32_t value );
void VM_LogSyscalls( uint32_t *args );

const char *VM_LoadInstructions( const byte *code_pos, uint32_t codeLength, uint32_t instructionCount, instruction_t *buf );
const char *VM_CheckInstructions( instruction_t *buf, uint32_t instructionCount,
								 const int32_t *jumpTableTargets,
								 uint32_t numJumpTableTargets,
								 uint32_t dataLength );
void VM_Free(vm_t* vm);
intptr_t VM_Call(vm_t *vm, uint32_t numArgs, int32_t command, ...);

#define JUMP	(1<<0)
#define FPU		(1<<1)

typedef struct opcode_info_s
{
	int	size;
	int	stack;
	int	nargs;
	int	flags;
} opcode_info_t;

extern opcode_info_t ops[ OP_MAX ];
extern vm_t vmTable[MAX_VM_COUNT];

int VM_MemoryRangeValid(intptr_t vmAddr, size_t len, vm_t* vm);
void VM_VmProfile_f(const vm_t* vm);
void VM_Debug(int level);

#ifdef __cplusplus
}
#endif

#endif

#endif /* __Q3VM_H */
