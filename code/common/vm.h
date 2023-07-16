#ifndef __Q3VM_H
#define __Q3VM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#ifdef _NOMAD_DEBUG
#define DEBUG_VM
#endif

#define VM_MAGIC 0x12721444
#define VM_PROGRAM_STACK_SIZE 0x10000
#define VM_MAX_IMAGE_SIZE 0x400000
#define VM_MAX_QPATH 64

#define VMA(x) VM_ArgPtr(args[x], vm)
#define VMF(x) VM_IntToFloat(args[x])

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

typedef struct
{
    int32_t vmMagic;          /**< Bytecode image shall start with VM_MAGIC */
    int32_t instructionCount; /**< Number of instructions in .qvm */
    int32_t codeOffset;       /**< Byte offset in .qvm file of .code segment */
    int32_t codeLength;       /**< Bytes in code segment */
    int32_t dataOffset;       /**< Byte offset in .qvm file of .data segment */
    int32_t dataLength;       /**< Bytes in .data segment */
    int32_t litLength; /**< Bytes in strings segment (after .data segment) */
    int32_t bssLength; /**< How many bytes should be used for .bss segment */
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

typedef struct vm_s
{
    vmHeader_t header;

    int32_t programStack;
    intptr_t (*systemCall)(struct vm_s* vm, intptr_t* parms);

    char  name[VM_MAX_QPATH];
    void* searchPath;

    void* unused_dllHandle;
    intptr_t (*unused_entryPoint)(int32_t callNum, ...);
    void (*unused_destroy)(struct vm_s* self);

    qboolean currentlyInterpreting;

    qboolean compiled;
    uint8_t* codeBase = NULL;
    int32_t entryOfs;
    uint32_t codeLength;

    intptr_t* instructionPointers = NULL;
    uint32_t instructionCount;

    uint8_t* dataBase = NULL;
    uint32_t dataMask;
    uint32_t dataAlloc;

    uint32_t stackBottom;

    uint32_t numSymbols;
    vmSymbol_t* symbols = NULL;

    uint32_t callLevel;
    uint32_t breakFunction;
    uint32_t breakCount;

    vmErrorCode_t lastError;
} vm_t;

void VM_Error(vm_t *vm, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));
void VM_Restart(vm_t *vm);


/** Initialize a virtual machine.
 * @param[out] vm Pointer to a virtual machine to initialize.
 * @param[in] module Path to the bytecode file. Used to load the
 *                   symbols. Otherwise not strictly required.
 * @param[in] bytecode Pointer to the bytecode. Directly byte by byte
 *                     the content of the .qvm file.
 * @param[in] length Number of bytes in the bytecode array.
 * @param[in] systemCalls Function pointer to callback function for native
 *   functions called by the bytecode. The function is identified by an integer
 *   id that corresponds to the bytecode function ids defined in g_syscalls.asm.
 *   Note however that parms equals to (-1 - function_id). So -1 in
 *   g_syscalls.asm equals to 0 in the systemCall parms argument, -2 in
 *   g_syscalls.asm is 1 in parms, -3 is 2 and so on.
 * @return 0 if everything is OK. -1 if something went wrong. */
int VM_Create(vm_t* vm, const char* module, const uint8_t* bytecode, int length,
              intptr_t (*systemCalls)(vm_t*, intptr_t*));

/** Free the memory of the virtual machine.
 * @param[in] vm Pointer to initialized virtual machine. */
void VM_Free(vm_t* vm);

/** Run a function from the virtual machine.
 * Use the command argument to tell the VM what to do.
 * You can supply additional (up to 12) parameters to pass to the bytecode.
 * @param[in] vm Pointer to initialized virtual machine.
 * @param[in] command Basic parameter passed to the bytecode.
 * @return Return value of the function call by the VM. */
intptr_t VM_Call(vm_t* vm, int command, ...);

/** Helper function for syscalls VMA(x) macro:
 * Translate from virtual machine memory to real machine memory.
 * If this is a memory range, use the VM_MemoryRangeValid() function to
 * make sure that this syscall does not escape from the sandbox.
 * @param[in] vmAddr Address in virtual machine memory
 * @param[in,out] vm Current VM
 * @return translated address. */
void* VM_ArgPtr(intptr_t vmAddr, vm_t* vm);

/** Helper function for syscalls VMF(x) macro:
 * Get argument in syscall and interpret it bit by bit as IEEE 754 float.
 * That is: just put the int in a float/int union and return the float.
 * If the VM calls a native function with a float argument: don't
 * cast the int argument to a float, but rather interpret it directly
 * as a floating point variable.
 * @param[in] x Argument on call stack.
 * @return Value as float. */
float VM_IntToFloat(int32_t x);

/** Helper function for syscalls:
 * Just put the float in a float/int union and return the int.
 * @param[in] f Floating point number.
 * @return Floating point number as integer */
int32_t VM_FloatToInt(float f);

/** Helper function for syscalls:
 * Check if address + range in in the valid VM memory range.
 * Use this function in the syscall callback to keep the VM in its sandbox.
 * @param[in] vmAddr Address in virtual machine memory
 * @param[in] len Length in bytes
 * @param[in] vm Current VM
 * @return 0 if valid (!), -1 if invalid. */
int VM_MemoryRangeValid(intptr_t vmAddr, size_t len, const vm_t* vm);

/** Print call statistics for every function. Only works with DEBUG_VM.
 * Does nothing if DEBUG_VM is not defined.
 * @param[in] vm VM to profile */
void VM_VmProfile_f(const vm_t* vm);

/** Set the printf debug level. Only useful with DEBUG_VM.
 * Set to 1 for general informations and 2 to output every opcode name.
 * @param[in] level If level is 0: be quiet (default). */
void VM_Debug(int level);

/******************************************************************************
 * CALLBACK FUNCTIONS (USER DEFINED IN HOST APPLICATION)
 ******************************************************************************/

#if 0
/** Implement this error callback function for error callbacks in the host
 * application.
 * @param[in] level Error identifier, see vmErrorCode_t.
 * @param[in] error Human readable error text. */
void Com_Error(vmErrorCode_t level, const char* error);

/** Implement this memory allocation function in the host application.
 * A simple implementation in the host app can just call malloc() or new[]
 * and ignore the vm and type parameters.
 * The type information can be used as a hint for static memory allocation
 * if needed.
 * @param[in] size Number of bytes to allocate.
 * @param[in] vm Pointer to vm requesting the memory.
 * @param[in] type What purpose has the requested memory, see vmMallocType_t.
 * @return pointer to allocated memory. */
void* Com_malloc(size_t size, vm_t* vm, vmMallocType_t type);

/** Implement this memory free function in the host application.
 * A simple implementation in the host app can just call free() or delete[]
 * and ignore the vm and type parameters.
 * @param[in,out] p Pointer of memory allocated by Com_malloc to be released.
 * @param[in] vm Pointer to vm releasing the memory.
 * @param[in] type What purpose has the memory, see vmMallocType_t. */
void Com_free(void* p, vm_t* vm, vmMallocType_t type);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __Q3VM_H */