#ifndef __Q3VM_H
#define __Q3VM_H

#ifndef Q3_VM

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

intptr_t VM_CallInterpreted(vm_t* vm, int command, ...);
int VM_CreateInterpreted(vm_t* vm, const char* module, const uint8_t* bytecode, int length,
              intptr_t (*systemCalls)(vm_t*, intptr_t*));

void VM_Free(vm_t* vm);
void* VM_ArgPtr(intptr_t vmAddr, vm_t* vm);
float VM_IntToFloat(int32_t x);
int32_t VM_FloatToInt(float f);
int VM_MemoryRangeValid(intptr_t vmAddr, size_t len, vm_t* vm);
void VM_VmProfile_f(const vm_t* vm);
void VM_Debug(int level);

#ifdef __cplusplus
}
#endif

#endif

#endif /* __Q3VM_H */
