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

#define VM_MAX_BSS_LENGTH 10485760

/******************************************************************************
 * TYPEDEFS
 ******************************************************************************/

/** Enum for the virtual machine op codes */
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

#ifndef USE_COMPUTED_GOTOS
/* for the the computed gotos we need labels,
 * but for the normal switch case we need the cases */
#define goto_OP_UNDEF case OP_UNDEF
#define goto_OP_IGNORE case OP_IGNORE
#define goto_OP_BREAK case OP_BREAK
#define goto_OP_ENTER case OP_ENTER
#define goto_OP_LEAVE case OP_LEAVE
#define goto_OP_CALL case OP_CALL
#define goto_OP_PUSH case OP_PUSH
#define goto_OP_POP case OP_POP
#define goto_OP_CONST case OP_CONST
#define goto_OP_LOCAL case OP_LOCAL
#define goto_OP_JUMP case OP_JUMP
#define goto_OP_EQ case OP_EQ
#define goto_OP_NE case OP_NE
#define goto_OP_LTI case OP_LTI
#define goto_OP_LEI case OP_LEI
#define goto_OP_GTI case OP_GTI
#define goto_OP_GEI case OP_GEI
#define goto_OP_LTU case OP_LTU
#define goto_OP_LEU case OP_LEU
#define goto_OP_GTU case OP_GTU
#define goto_OP_GEU case OP_GEU
#define goto_OP_EQF case OP_EQF
#define goto_OP_NEF case OP_NEF
#define goto_OP_LTF case OP_LTF
#define goto_OP_LEF case OP_LEF
#define goto_OP_GTF case OP_GTF
#define goto_OP_GEF case OP_GEF
#define goto_OP_LOAD1 case OP_LOAD1
#define goto_OP_LOAD2 case OP_LOAD2
#define goto_OP_LOAD4 case OP_LOAD4
#define goto_OP_STORE1 case OP_STORE1
#define goto_OP_STORE2 case OP_STORE2
#define goto_OP_STORE4 case OP_STORE4
#define goto_OP_ARG case OP_ARG
#define goto_OP_BLOCK_COPY case OP_BLOCK_COPY
#define goto_OP_SEX8 case OP_SEX8
#define goto_OP_SEX16 case OP_SEX16
#define goto_OP_NEGI case OP_NEGI
#define goto_OP_ADD case OP_ADD
#define goto_OP_SUB case OP_SUB
#define goto_OP_DIVI case OP_DIVI
#define goto_OP_DIVU case OP_DIVU
#define goto_OP_MODI case OP_MODI
#define goto_OP_MODU case OP_MODU
#define goto_OP_MULI case OP_MULI
#define goto_OP_MULU case OP_MULU
#define goto_OP_BAND case OP_BAND
#define goto_OP_BOR case OP_BOR
#define goto_OP_BXOR case OP_BXOR
#define goto_OP_BCOM case OP_BCOM
#define goto_OP_LSH case OP_LSH
#define goto_OP_RSHI case OP_RSHI
#define goto_OP_RSHU case OP_RSHU
#define goto_OP_NEGF case OP_NEGF
#define goto_OP_ADDF case OP_ADDF
#define goto_OP_SUBF case OP_SUBF
#define goto_OP_DIVF case OP_DIVF
#define goto_OP_MULF case OP_MULF
#define goto_OP_CVIF case OP_CVIF
#define goto_OP_CVFI case OP_CVFI
#endif

/******************************************************************************
 * LOCAL DATA DEFINITIONS
 ******************************************************************************/

static int vm_debugLevel; /**< 0: be quiet, 1: debug msgs, 2: print op codes */

#ifdef DEBUG_VM
/** Table to convert op codes to readable names */
const static char* opnames[OPCODE_TABLE_SIZE] = {
    "OP_UNDEF",  "OP_IGNORE", "OP_BREAK",  "OP_ENTER", "OP_LEAVE",
    "OP_CALL",   "OP_PUSH",   "OP_POP",    "OP_CONST", "OP_LOCAL",
    "OP_JUMP",   "OP_EQ",     "OP_NE",     "OP_LTI",   "OP_LEI",
    "OP_GTI",    "OP_GEI",    "OP_LTU",    "OP_LEU",   "OP_GTU",
    "OP_GEU",    "OP_EQF",    "OP_NEF",    "OP_LTF",   "OP_LEF",
    "OP_GTF",    "OP_GEF",    "OP_LOAD1",  "OP_LOAD2", "OP_LOAD4",
    "OP_STORE1", "OP_STORE2", "OP_STORE4", "OP_ARG",   "OP_BLOCK_COPY",
    "OP_SEX8",   "OP_SEX16",  "OP_NEGI",   "OP_ADD",   "OP_SUB",
    "OP_DIVI",   "OP_DIVU",   "OP_MODI",   "OP_MODU",  "OP_MULI",
    "OP_MULU",   "OP_BAND",   "OP_BOR",    "OP_BXOR",  "OP_BCOM",
    "OP_LSH",    "OP_RSHI",   "OP_RSHU",   "OP_NEGF",  "OP_ADDF",
    "OP_SUBF",   "OP_DIVF",   "OP_MULF",   "OP_CVIF",  "OP_CVFI",
    "OP_UNDEF",  "OP_UNDEF",  "OP_UNDEF",  "OP_UNDEF",
};
#endif

/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 ******************************************************************************/

static const vmHeader_t* VM_LoadQVM(vm_t* vm, const uint8_t* bytecode,
                                    int length);
static int VM_PrepareInterpreter(vm_t* vm, const vmHeader_t* header);
static int VM_CallInterpreted2(vm_t* vm, int* args);
static void VM_BlockCopy(unsigned int dest, unsigned int src, size_t n,
                         vm_t* vm);
extern float _vmf(intptr_t x);


#ifdef DEBUG_VM
static char* VM_Indent(vm_t* vm);
static vmSymbol_t* VM_ValueToFunctionSymbol(vm_t* vm, int value);
static const char* VM_ValueToSymbol(vm_t* vm, int value);
static void VM_LoadSymbols(vm_t* vm);
static uint8_t* loadImage(const char* filepath, int* imageSize);
static void VM_StackTrace(vm_t* vm, int programCounter, int programStack);
#endif

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

void VM_Restart(vm_t *vm)
{
    Con_Printf("VM_Restart: restarting vm %s", vm->name);

    VM_Free(vm);
    uint8_t *bytecode;
    uint64_t codeLength = FS_LoadFile(va("%s.qvm", vm->name), (void **)&bytecode);
    const vmHeader_t *header = VM_LoadQVM(vm, bytecode, codeLength);
    FS_FreeFile(bytecode);
    if (!header) {
        N_Error("VM_Restart: failed to load qvm bytecode for %s", vm->name);
    }

    vm->instructionCount = header->instructionCount;
    vm->codeLength = header->codeLength;

    vm->compiled = qfalse;
    if (VM_PrepareInterpreter(vm, header) != 0) {
        N_Error("VM_Restart: VM_PrepareInterpreter failed");
    }

    vm->programStack = vm->dataMask + 1;
    vm->stackBottom = vm->programStack - VM_PROGRAM_STACK_SIZE;
}

/******************************************************************************
 * LOCAL INLINE FUNCTIONS AND FUNCTION MACROS
 ******************************************************************************/

#define Q_ftol(v) ((long)(v))

/******************************************************************************
 * FUNCTION BODIES
 ******************************************************************************/

int VM_CreateInterpreted(vm_t* vm, const char* name, const uint8_t* bytecode, int length,
              intptr_t (*systemCalls)(vm_t*, intptr_t*))
{
    if (!vm)
        N_Error("VM_CreateInterpreted: NULL vm");
    if (!systemCalls)
        N_Error("VM_CreateInterpreted: NULL systemCalls");

    memset(vm, 0, sizeof(vm_t));
    N_strncpyz(vm->name, name, sizeof(vm->name));
    const vmHeader_t* header = VM_LoadQVM(vm, bytecode, length);
    if (!header)
        N_Error("Failed to load bytecode");

    vm->systemCall = systemCalls;

    /* allocate space for the jump targets, which will be filled in by the
       compile/prep functions */
    vm->instructionCount    = header->instructionCount;

    if (!vm->instructionPointers)
        vm->instructionPointers = (intptr_t *)Hunk_Alloc(vm->instructionCount * sizeof(*vm->instructionPointers),
            "instructionPointers", h_low);

    vm->codeLength = header->codeLength;

    vm->compiled = qfalse; /* no JIT */
    if (!vm->compiled) {
        if (VM_PrepareInterpreter(vm, header) != 0) {
            VM_Free(vm);
            return -1;
        }
    }

#ifdef DEBUG_VM
    /* load the map file */
    VM_LoadSymbols(vm);
#endif

    /* the stack is implicitly at the end of the image */
    vm->programStack = vm->dataMask + 1;
    vm->stackBottom  = vm->programStack - VM_PROGRAM_STACK_SIZE;

#ifdef DEBUG_VM
    Con_Printf("VM:");
    Con_Printf(".code length: %6i bytes", header->codeLength);
    Con_Printf(".data length: %6i bytes", header->dataLength);
    Con_Printf(".lit  length: %6i bytes", header->litLength);
    Con_Printf(".bss  length: %6i bytes", header->bssLength);
    Con_Printf("Stack size:   %6i bytes", VM_PROGRAM_STACK_SIZE);
    Con_Printf("Allocated memory: %6i bytes", vm->dataAlloc);
    Con_Printf("Instruction count: %i", header->instructionCount);
#endif

    return 0;
}

static const vmHeader_t* VM_LoadQVM(vm_t* vm, const uint8_t* bytecode, int length)
{
    int dataLength;
    int i;
    const union {
        const vmHeader_t* h;
        const uint8_t*    v;
    } header = {.v = bytecode };

    Con_Printf("Loading vm file %s...", vm->name);

    if (!header.h || !bytecode || length <= (int)sizeof(vmHeader_t) || length > VM_MAX_IMAGE_SIZE)
        N_Error("VM_CreateInterpreted: failed on '%s'", vm->name);

    if (LittleInt(header.h->vmMagic) == VM_MAGIC) {
        /* byte swap the header */
        for (i = 0; i < (int)(sizeof(vmHeader_t)) / 4; i++) {
            ((int*)header.h)[i] = LittleInt(((int*)header.h)[i]);
        }

        /* validate */
        if (header.h->bssLength < 0 || header.h->dataLength < 0 ||
            header.h->litLength < 0 || header.h->codeLength <= 0 ||
            header.h->codeOffset < 0 || header.h->dataOffset < 0 ||
            header.h->instructionCount <= 0 ||
            header.h->bssLength > VM_MAX_BSS_LENGTH ||
            header.h->codeOffset + header.h->codeLength > length ||
            header.h->dataOffset + header.h->dataLength + header.h->litLength > length) {
            Con_Printf(WARNING, "%s has bad header", vm->name);
            return NULL;
        }
    }
    else {
        Con_Printf(WARNING, "Invalid magic number in header of \"%s\". "
                   "Read: 0x%x, expected: 0x%x",
                   vm->name, LittleInt(header.h->vmMagic), VM_MAGIC);
        return NULL;
    }

    /* round up to next power of 2 so all data operations can
       be mask protected */
    dataLength = header.h->dataLength + header.h->litLength + header.h->bssLength;
    for (i = 0; dataLength > (1 << i); i++) { }
    dataLength = 1 << i;

    /* allocate zero filled space for initialized and uninitialized data
     leave some space beyond data mask so we can secure all mask operations */
    vm->dataAlloc = dataLength + 4;
    if (!vm->dataBase)
        vm->dataBase  = (uint8_t *)Hunk_Alloc(vm->dataAlloc, "vmDataBase", h_low);
    
    vm->dataMask  = dataLength - 1;

    /* make sure data section is always initialized with 0
     * (bss would be enough) */
    memset(vm->dataBase, 0, vm->dataAlloc);

    /* copy the intialized data */
    memcpy(vm->dataBase, header.v + header.h->dataOffset, header.h->dataLength + header.h->litLength);

    /* byte swap the longs */
    for (i = 0; i < header.h->dataLength; i += sizeof(int)) {
        *(int *)(vm->dataBase + i) = LittleInt(*(int*)(vm->dataBase + i));
    }

    return header.h;
}

intptr_t VM_CallInterpreted(vm_t* vm, int numargs, int command, ...)
{
    intptr_t r;
    int      args[MAX_VMMAIN_ARGS];
    va_list  ap;
    int      i;

    if (!vm)
        N_Error("VM_CallInterpreted on null vm");
    if (vm->codeLength < 1)
        N_Error("VM_CallInterpreted: vm not yet loaded");

    args[0] = command;
    va_start(ap, command);
    for (i = 1; i < numargs; i++) {
        args[i] = va_arg(ap, int);
    }
    va_end(ap);

    ++vm->callLevel;
    r = VM_CallInterpreted2(vm, args);
    --vm->callLevel;

    return r;
}

void VM_Free(vm_t* vm)
{
    if (!vm)
        return;
    if (vm->callLevel)
        N_Error("VM_Free: vm free on running vm!");

    // everything will be automatically deallocated by the hunk
    if (vm->codeBase)
        memset(vm->codeBase, 0, vm->codeLength * 4);
    if (vm->dataBase)
        memset(vm->dataBase, 0, vm->dataAlloc);
    if (vm->instructionPointers)
        memset(vm->instructionPointers, 0, vm->instructionCount * sizeof(*vm->instructionPointers));

#ifdef DEBUG_VM
    vmSymbol_t* sym = vm->symbols;
    while (sym) {
        vmSymbol_t* next = sym->next;
        Z_Free(sym);
        sym = next;
    }
#endif

    memset(vm, 0, sizeof(*vm));
}

void* VM_ArgPtr(intptr_t vmAddr, vm_t* vm)
{
    if (!vmAddr)
        return NULL;
    if (!vm)
        N_Error("VM_ArgPtr: VM is null");

    return (void*)(vm->dataBase + (vmAddr & vm->dataMask));
}

float VM_IntToFloat(int32_t x)
{
    union {
        float    f;  /**< float IEEE 754 32-bit single */
        int32_t  i;  /**< int32 part */
        uint32_t ui; /**< unsigned int32 part */
    } fi;
    fi.i = x;
    return fi.f;
}

int32_t VM_FloatToInt(float f)
{
    union {
        float    f;  /**< float IEEE 754 32-bit single */
        int32_t  i;  /**< int32 part */
        uint32_t ui; /**< unsigned int32 part */
    } fi;
    fi.f = f;
    return fi.i;
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
        Con_Error(false,
                  "OP_BLOCK_COPY out of range");
        return;
    }

    memcpy(vm->dataBase + dest, vm->dataBase + src, n);
}

static int VM_PrepareInterpreter(vm_t* vm, const vmHeader_t* header)
{
    int      op;
    int      byte_pc;
    int      int_pc;
    uint8_t* code;
    int      instruction;
    int*     codeBase;

    if (!vm->codeBase)
        vm->codeBase = (uint8_t *)Hunk_Alloc(vm->codeLength * 4, "codeBase", h_low);
    
    memcpy(vm->codeBase, (uint8_t*)header + header->codeOffset, vm->codeLength);

    /* we don't need to translate the instructions, but we still need
       to find each instructions starting point for jumps */
    int_pc = byte_pc = 0;
    instruction      = 0;
    code             = (uint8_t*)header + header->codeOffset;
    codeBase         = (int*)vm->codeBase;

    /* Copy and expand instructions to words while
     * building instruction table */
    while (instruction < header->instructionCount) {
        vm->instructionPointers[instruction] = int_pc;
        instruction++;

        op               = (int)code[byte_pc];
        codeBase[int_pc] = op;
        if (byte_pc > header->codeLength) {
            Con_Error(false, "VM_PrepareInterpreter: pc > header->codeLength");
            return -1;
        }

        byte_pc++;
        int_pc++;

        /* these are the only opcodes that aren't a single byte */
        switch (op) {
        case OP_ENTER:
        case OP_CONST:
        case OP_LOCAL:
        case OP_LEAVE:
        case OP_EQ:
        case OP_NE:
        case OP_LTI:
        case OP_LEI:
        case OP_GTI:
        case OP_GEI:
        case OP_LTU:
        case OP_LEU:
        case OP_GTU:
        case OP_GEU:
        case OP_EQF:
        case OP_NEF:
        case OP_LTF:
        case OP_LEF:
        case OP_GTF:
        case OP_GEF:
        case OP_BLOCK_COPY:
            codeBase[int_pc] = LittleInt(code[byte_pc]);
            byte_pc += 4;
            int_pc++;
            break;
        case OP_ARG:
            codeBase[int_pc] = (int)code[byte_pc];
            byte_pc++;
            int_pc++;
            break;
        default:
            if (op < 0 || op >= OP_MAX) {
                Con_Error(false, "Bad VM instruction");
                return -1;
            }
            break;
        };
    }
    int_pc      = 0;
    instruction = 0;

    /* Now that the code has been expanded to int-sized opcodes, we'll translate
       instruction index
       into an index into codeBase[], which contains opcodes and operands. */
    while (instruction < header->instructionCount) {
        op = codeBase[int_pc];
        instruction++;
        int_pc++;

        switch (op) {
        /* These ops need to translate addresses in jumps from instruction index
           to int index */
        case OP_EQ:
        case OP_NE:
        case OP_LTI:
        case OP_LEI:
        case OP_GTI:
        case OP_GEI:
        case OP_LTU:
        case OP_LEU:
        case OP_GTU:
        case OP_GEU:
        case OP_EQF:
        case OP_NEF:
        case OP_LTF:
        case OP_LEF:
        case OP_GTF:
        case OP_GEF:
            if (codeBase[int_pc] < 0 || codeBase[int_pc] > vm->instructionCount) {
                Con_Error(false,
                          "VM_PrepareInterpreter: Jump to invalid "
                          "instruction number");
                return -1;
            }

            /* codeBase[pc] is the instruction index. Convert that into a
               offset into
               the int-aligned codeBase[] by the lookup table. */
            codeBase[int_pc] = vm->instructionPointers[codeBase[int_pc]];
            int_pc++;
            break;
        /* These opcodes have an operand that isn't an instruction index */
        case OP_ENTER:
        case OP_CONST:
        case OP_LOCAL:
        case OP_LEAVE:
        case OP_BLOCK_COPY:
        case OP_ARG:
            int_pc++;
            break;

        default:
            break;
        };
    }
    return 0;
}

/*
==============
VM_CallInterpreted2

Upon a system call, the stack will look like:

sp+32   parm1
sp+28   parm0
sp+24   return stack
sp+20   return address
sp+16   local1
sp+14   local0
sp+12   arg1
sp+8    arg0
sp+4    return stack
sp      return address

An interpreted function will immediately execute
an OP_ENTER instruction, which will subtract space for
locals from sp
==============
*/

static int VM_CallInterpreted2(vm_t* vm, int* args)
{
    uint8_t  stack[OPSTACK_SIZE + 15];
    int*     opStack;
    uint8_t  opStackOfs;
    int      programCounter;
    int      programStack;
    int      stackOnEntry;
    uint8_t* image;
    int*     codeImage;
    int      v1;
    int      dataMask;
    int      arg;
#ifdef DEBUG_VM
    vmSymbol_t* profileSymbol;
#endif

    /* interpret the code */
    vm->currentlyInterpreting = qtrue;

    /* we might be called recursively, so this might not be the very top */
    programStack = stackOnEntry = vm->programStack;

#ifdef DEBUG_VM
    profileSymbol = VM_ValueToFunctionSymbol(vm, 0);
    /* uncomment this for debugging breakpoints */
    vm->breakFunction = 0;
#endif

    image          = vm->dataBase;
    codeImage      = (int*)vm->codeBase;
    dataMask       = vm->dataMask;
    programCounter = 0;
    programStack -= (8 + 4 * MAX_VMMAIN_ARGS);

    for (arg = 0; arg < MAX_VMMAIN_ARGS; arg++)
    {
        *(int*)&image[programStack + 8 + arg * 4] = args[arg];
    }

    *(int*)&image[programStack + 4] = 0; /* return stack */
    *(int*)&image[programStack] = -1;    /* will terminate the loop on return */

    /* leave a free spot at start of stack so
       that as long as opStack is valid, opStack-1 will
       not corrupt anything */
    opStack    = (int *)PADP(stack, 16);
    *opStack   = 0x0000BEEF;
    opStackOfs = 0;

    /* main interpreter loop, will exit when a LEAVE instruction
       grabs the -1 program counter */

    int opcode, r0, r1;
#define r2 codeImage[programCounter]

#ifdef USE_COMPUTED_GOTOS
    static const void* dispatch_table[OPCODE_TABLE_SIZE] = {
        &&goto_OP_UNDEF,  &&goto_OP_IGNORE,     &&goto_OP_BREAK,
        &&goto_OP_ENTER,  &&goto_OP_LEAVE,      &&goto_OP_CALL,
        &&goto_OP_PUSH,   &&goto_OP_POP,        &&goto_OP_CONST,
        &&goto_OP_LOCAL,  &&goto_OP_JUMP,       &&goto_OP_EQ,
        &&goto_OP_NE,     &&goto_OP_LTI,        &&goto_OP_LEI,
        &&goto_OP_GTI,    &&goto_OP_GEI,        &&goto_OP_LTU,
        &&goto_OP_LEU,    &&goto_OP_GTU,        &&goto_OP_GEU,
        &&goto_OP_EQF,    &&goto_OP_NEF,        &&goto_OP_LTF,
        &&goto_OP_LEF,    &&goto_OP_GTF,        &&goto_OP_GEF,
        &&goto_OP_LOAD1,  &&goto_OP_LOAD2,      &&goto_OP_LOAD4,
        &&goto_OP_STORE1, &&goto_OP_STORE2,     &&goto_OP_STORE4,
        &&goto_OP_ARG,    &&goto_OP_BLOCK_COPY, &&goto_OP_SEX8,
        &&goto_OP_SEX16,  &&goto_OP_NEGI,       &&goto_OP_ADD,
        &&goto_OP_SUB,    &&goto_OP_DIVI,       &&goto_OP_DIVU,
        &&goto_OP_MODI,   &&goto_OP_MODU,       &&goto_OP_MULI,
        &&goto_OP_MULU,   &&goto_OP_BAND,       &&goto_OP_BOR,
        &&goto_OP_BXOR,   &&goto_OP_BCOM,       &&goto_OP_LSH,
        &&goto_OP_RSHI,   &&goto_OP_RSHU,       &&goto_OP_NEGF,
        &&goto_OP_ADDF,   &&goto_OP_SUBF,       &&goto_OP_DIVF,
        &&goto_OP_MULF,   &&goto_OP_CVIF,       &&goto_OP_CVFI,
        &&goto_OP_UNDEF, /* Invalid OP CODE for opcode_table_mask */
        &&goto_OP_UNDEF, /* Invalid OP CODE for opcode_table_mask */
        &&goto_OP_UNDEF, /* Invalid OP CODE for opcode_table_mask */
        &&goto_OP_UNDEF, /* Invalid OP CODE for opcode_table_mask */
    };
#define DISPATCH2()                                                            \
    opcode = codeImage[programCounter++];                                      \
    goto* dispatch_table[opcode & OPCODE_TABLE_MASK]
#define DISPATCH()                                                             \
    r0 = opStack[opStackOfs];                                                  \
    r1 = opStack[(uint8_t)(opStackOfs - 1)];                                   \
    DISPATCH2()
    DISPATCH(); /* initial jump into the loop */
#else
#define DISPATCH2() goto nextInstruction2
#define DISPATCH() goto nextInstruction
#endif

    while (1) {
#ifndef USE_COMPUTED_GOTOS
    nextInstruction:
        r0 = opStack[opStackOfs];
        r1 = opStack[(uint8_t)(opStackOfs - 1)];
    nextInstruction2:
        opcode = codeImage[programCounter++];

#ifdef DEBUG_VM
        if ((unsigned)programCounter >= vm->codeLength) {
            VM_Error(vm, "VM pc out of range");
            return -1;
        }
        if (programStack <= vm->stackBottom) {
            VM_Error(vm, "VM stack overflow");
            return -1;
        }
        if (programStack & 3) {
            VM_Error(vm, "VM program stack misaligned");
            return -1;
        }

        if (vm_debugLevel > 1) {
            Con_Printf("%s%i %s", VM_Indent(vm), opStackOfs,
                       opnames[opcode & OPCODE_TABLE_MASK]);
        }
        profileSymbol->profileCount++;
#endif /* DEBUG_VM */
        switch (opcode)
#endif /* !USE_COMPUTED_GOTOS */
        {
#ifdef DEBUG_VM
        default: /* fall through */
#endif
        goto_OP_UNDEF:
            VM_Error(vm, "Bad VM instruction");
            return -1;
        goto_OP_IGNORE:
            DISPATCH2();
        goto_OP_BREAK:
            vm->breakCount++;
            DISPATCH2();
        goto_OP_CONST:
            opStackOfs++;
            r1 = r0;
            r0 = opStack[opStackOfs] = r2;

            programCounter += 1;
            DISPATCH2();
        goto_OP_LOCAL:
            opStackOfs++;
            r1 = r0;
            r0 = opStack[opStackOfs] = r2 + programStack;

            programCounter += 1;
            DISPATCH2();
        goto_OP_LOAD4:
#ifdef DEBUG_VM
            if (opStack[opStackOfs] & 3) {
                VM_Error(vm, "OP_LOAD4 misaligned");
                return -1;
            }
#endif
            r0 = opStack[opStackOfs] = *(int*)&image[r0 & dataMask];
            DISPATCH2();
        goto_OP_LOAD2:
            r0 = opStack[opStackOfs] = *(unsigned short*)&image[r0 & dataMask];
            DISPATCH2();
        goto_OP_LOAD1:
            r0 = opStack[opStackOfs] = image[r0 & dataMask];
            DISPATCH2();

        goto_OP_STORE4:
            *(int*)&image[r1 & dataMask] = r0;
            opStackOfs -= 2;
            DISPATCH();
        goto_OP_STORE2:
            *(short*)&image[r1 & dataMask] = r0;
            opStackOfs -= 2;
            DISPATCH();
        goto_OP_STORE1:
            image[r1 & dataMask] = r0;
            opStackOfs -= 2;
            DISPATCH();
        goto_OP_ARG:
            /* single byte offset from programStack */
            *(int*)&image[(codeImage[programCounter] + programStack) &
                          dataMask] = r0;
            opStackOfs--;
            programCounter += 1;
            DISPATCH();
        goto_OP_BLOCK_COPY:
            VM_BlockCopy(r1, r0, r2, vm);
            programCounter += 1;
            opStackOfs -= 2;
            DISPATCH();
        goto_OP_CALL:
            /* save current program counter */
            *(int*)&image[programStack] = programCounter;

            /* jump to the location on the stack */
            programCounter = r0;
            opStackOfs--;
            if (programCounter < 0) { /* system call */
                int r;
#ifdef DEBUG_VM
                if (vm_debugLevel) {
                    Con_Printf("%s%i---> systemcall(%i)", VM_Indent(vm),
                               opStackOfs, -1 - programCounter);
                }
#endif
                /* save the stack to allow recursive VM entry */
                vm->programStack = programStack - 4;
#ifdef DEBUG_VM
                int stomped = *(int*)&image[programStack + 4];
#endif
                *(int*)&image[programStack + 4] = -1 - programCounter;

                /* the vm has ints on the stack, we expect
                   pointers so we might have to convert it */
                if (sizeof(intptr_t) != sizeof(int)) {
                    intptr_t argarr[MAX_VMSYSCALL_ARGS];
                    int*     imagePtr = (int*)&image[programStack];
                    int      i;
                    for (i = 0; i < (int)arraylen(argarr); ++i) {
                        argarr[i] = *(++imagePtr);
                    }
                    r = vm->systemCall(vm, argarr);
                }
                else {
                    r = vm->systemCall(vm, (intptr_t*)&image[programStack + 4]);
                }

#ifdef DEBUG_VM
                /* this is just our stack frame pointer, only needed
                   for debugging */
                *(int*)&image[programStack + 4] = stomped;
#endif

                /* save return value */
                opStackOfs++;
                opStack[opStackOfs] = r;
                programCounter      = *(int*)&image[programStack];
#ifdef DEBUG_VM
                if (vm_debugLevel) {
                    Con_Printf("%s%i<--- %s", VM_Indent(vm), opStackOfs,
                               VM_ValueToSymbol(vm, programCounter));
                }
#endif
            }
            else if ((unsigned)programCounter >= (unsigned)vm->instructionCount) {
                VM_Error(vm, "VM program counter out of range in OP_CALL");
                return -1;
            }
            else {
                programCounter = vm->instructionPointers[programCounter];
            }
            DISPATCH();
        /* push and pop are only needed for discarded or bad function return
           values */
        goto_OP_PUSH:
            opStackOfs++;
            DISPATCH();
        goto_OP_POP:
            opStackOfs--;
            DISPATCH();
        goto_OP_ENTER:
            /* get size of stack frame */
            v1 = r2;

            programCounter += 1;
            programStack -= v1;
#ifdef DEBUG_VM
            profileSymbol = VM_ValueToFunctionSymbol(vm, programCounter);
            /* save old stack frame for debugging traces */
            *(int*)&image[programStack + 4] = programStack + v1;
            if (vm_debugLevel) {
                Con_Printf("%s%i---> %s", VM_Indent(vm), opStackOfs,
                           VM_ValueToSymbol(vm, programCounter - 5));
                if (vm->breakFunction &&
                    programCounter - 5 == vm->breakFunction) {
                    /* this is to allow setting breakpoints here in the
                     * debugger */
                    vm->breakCount++;
                    VM_StackTrace(vm, programCounter, programStack);
                }
            }
#endif
            DISPATCH();
        goto_OP_LEAVE:
            /* remove our stack frame */
            v1 = r2;

            programStack += v1;

            /* grab the saved program counter */
            programCounter = *(int*)&image[programStack];
#ifdef DEBUG_VM
            profileSymbol = VM_ValueToFunctionSymbol(vm, programCounter);
            if (vm_debugLevel) {
                Con_Printf("%s%i<--- %s", VM_Indent(vm), opStackOfs,
                           VM_ValueToSymbol(vm, programCounter));
            }
#endif
            /* check for leaving the VM */
            if (programCounter == -1) {
                goto done;
            }
            else if ((unsigned)programCounter >= (unsigned)vm->codeLength) {
                VM_Error(vm, "VM program counter out of range in OP_LEAVE");
                return -1;
            }
            DISPATCH();

        /*
           ===================================================================
           BRANCHES
           ===================================================================
           */

        goto_OP_JUMP:
            if ((unsigned)r0 >= (unsigned)vm->instructionCount) {
                VM_Error(vm, "VM program counter out of range in OP_JUMP");
                return -1;
            }

            programCounter = vm->instructionPointers[r0];

            opStackOfs--;
            DISPATCH();
        goto_OP_EQ:
            opStackOfs -= 2;
            if (r1 == r0) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_NE:
            opStackOfs -= 2;
            if (r1 != r0) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LTI:
            opStackOfs -= 2;
            if (r1 < r0) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LEI:
            opStackOfs -= 2;
            if (r1 <= r0) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GTI:
            opStackOfs -= 2;
            if (r1 > r0) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GEI:
            opStackOfs -= 2;
            if (r1 >= r0) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LTU:
            opStackOfs -= 2;
            if (((unsigned)r1) < ((unsigned)r0)) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LEU:
            opStackOfs -= 2;
            if (((unsigned)r1) <= ((unsigned)r0)) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GTU:
            opStackOfs -= 2;
            if (((unsigned)r1) > ((unsigned)r0)) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GEU:
            opStackOfs -= 2;
            if (((unsigned)r1) >= ((unsigned)r0)) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_EQF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)(opStackOfs + 1)] ==
                ((float*)opStack)[(uint8_t)(opStackOfs + 2)]) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_NEF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)(opStackOfs + 1)] !=
                ((float*)opStack)[(uint8_t)(opStackOfs + 2)]) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LTF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)(opStackOfs + 1)] <
                ((float*)opStack)[(uint8_t)(opStackOfs + 2)]) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_LEF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)((uint8_t)(opStackOfs + 1))] <=
                ((float*)opStack)[(uint8_t)((uint8_t)(opStackOfs + 2))]) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GTF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)(opStackOfs + 1)] >
                ((float*)opStack)[(uint8_t)(opStackOfs + 2)]) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }
        goto_OP_GEF:
            opStackOfs -= 2;

            if (((float*)opStack)[(uint8_t)(opStackOfs + 1)] >=
                ((float*)opStack)[(uint8_t)(opStackOfs + 2)]) {
                programCounter = r2; /* vm->instructionPointers[r2]; */
                DISPATCH();
            }
            else {
                programCounter += 1;
                DISPATCH();
            }

        /*===================================================================*/

        goto_OP_NEGI:
            opStack[opStackOfs] = -r0;
            DISPATCH();
        goto_OP_ADD:
            opStackOfs--;
            opStack[opStackOfs] = r1 + r0;
            DISPATCH();
        goto_OP_SUB:
            opStackOfs--;
            opStack[opStackOfs] = r1 - r0;
            DISPATCH();
        goto_OP_DIVI:
            opStackOfs--;
            opStack[opStackOfs] = r1 / r0;
            DISPATCH();
        goto_OP_DIVU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) / ((unsigned)r0);
            DISPATCH();
        goto_OP_MODI:
            opStackOfs--;
            opStack[opStackOfs] = r1 % r0;
            DISPATCH();
        goto_OP_MODU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) % ((unsigned)r0);
            DISPATCH();
        goto_OP_MULI:
            opStackOfs--;
            opStack[opStackOfs] = r1 * r0;
            DISPATCH();
        goto_OP_MULU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) * ((unsigned)r0);
            DISPATCH();
        goto_OP_BAND:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) & ((unsigned)r0);
            DISPATCH();
        goto_OP_BOR:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) | ((unsigned)r0);
            DISPATCH();
        goto_OP_BXOR:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) ^ ((unsigned)r0);
            DISPATCH();
        goto_OP_BCOM:
            opStack[opStackOfs] = ~((unsigned)r0);
            DISPATCH();
        goto_OP_LSH:
            opStackOfs--;
            opStack[opStackOfs] = r1 << r0;
            DISPATCH();
        goto_OP_RSHI:
            opStackOfs--;
            opStack[opStackOfs] = r1 >> r0;
            DISPATCH();
        goto_OP_RSHU:
            opStackOfs--;
            opStack[opStackOfs] = ((unsigned)r1) >> r0;
            DISPATCH();
        goto_OP_NEGF:
            ((float*)opStack)[opStackOfs] = -((float*)opStack)[opStackOfs];
            DISPATCH();
        goto_OP_ADDF:
            opStackOfs--;
            ((float*)opStack)[opStackOfs] =
                ((float*)opStack)[opStackOfs] +
                ((float*)opStack)[(uint8_t)(opStackOfs + 1)];
            DISPATCH();
        goto_OP_SUBF:
            opStackOfs--;
            ((float*)opStack)[opStackOfs] =
                ((float*)opStack)[opStackOfs] -
                ((float*)opStack)[(uint8_t)(opStackOfs + 1)];
            DISPATCH();
        goto_OP_DIVF:
            opStackOfs--;
            ((float*)opStack)[opStackOfs] =
                ((float*)opStack)[opStackOfs] /
                ((float*)opStack)[(uint8_t)(opStackOfs + 1)];
            DISPATCH();
        goto_OP_MULF:
            opStackOfs--;
            ((float*)opStack)[opStackOfs] =
                ((float*)opStack)[opStackOfs] *
                ((float*)opStack)[(uint8_t)(opStackOfs + 1)];
            DISPATCH();
        goto_OP_CVIF:
            ((float*)opStack)[opStackOfs] = (float)opStack[opStackOfs];
            DISPATCH();
        goto_OP_CVFI:
            opStack[opStackOfs] = Q_ftol(((float*)opStack)[opStackOfs]);
            DISPATCH();
        goto_OP_SEX8:
            opStack[opStackOfs] = (int8_t)opStack[opStackOfs];
            DISPATCH();
        goto_OP_SEX16:
            opStack[opStackOfs] = (int16_t)opStack[opStackOfs];
            DISPATCH();
        }
    }

done:
    vm->currentlyInterpreting = qfalse;

    if (opStackOfs != 1 || *opStack != 0x0000BEEF) {
        N_Error("Interpreter stack error");
    }

    vm->programStack = stackOnEntry;

    /* return the result of the bytecode computations */
    return opStack[opStackOfs];
}

/* DEBUG FUNCTIONS */
/* --------------- */

void VM_Debug(int level)
{
    vm_debugLevel = level;
}

#ifdef DEBUG_VM
static char* VM_Indent(vm_t* vm)
{
    static char* string = "                                        ";
    if (vm->callLevel > 20) {
        return string;
    }
    return string + 2 * (20 - vm->callLevel);
}

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


uint8_t* loadImage(const char* filepath, int* size)
{
    file_t   f;
    uint8_t* image = NULL; /* bytecode buffer */
    int      sz;           /* bytecode file size */

    *size = 0;
    f = FS_FOpenRead(filepath);
    if (f == FS_INVALID_HANDLE) {
        Con_Error(false, "Failed to open file %s", filepath);
        return NULL;
    }
    sz = FS_FileLength(f);
    if (sz < 1) {
        FS_FClose(f);
        return NULL;
    }

    image = (uint8_t *)Z_Malloc(sz, TAG_STATIC, &image, "filebuffer");

    if (FS_Read(image, sz, f) != (size_t)sz) {
        Z_Free(image);
        FS_FClose(f);
        return NULL;
    }

    FS_FClose(f);
    *size = sz;
    return image;
}

static void VM_LoadSymbols(vm_t* vm)
{
    union {
        char* c;
        void* v;
    } mapfile;
    const char  *text_p, *token;
    char         name[MAX_GDR_PATH];
    char         symbols[MAX_GDR_PATH];
    vmSymbol_t **prev, *sym;
    int          count;
    int          value;
    int          chars;
    int          segment;
    int          numInstructions;
    int          imageSize;

    COM_StripExtension(vm->name, name, MAX_GDR_PATH);
    snprintf(symbols, sizeof(symbols), "%s.map", name);
    Con_Printf("Loading symbol file: %s...", symbols);
    mapfile.v = loadImage(symbols, &imageSize);

    if (!mapfile.c) {
        Con_Printf("Couldn't load symbol file: %s", symbols);
        return;
    }

    numInstructions = vm->instructionCount;

    /* parse the symbols */
    text_p = mapfile.c;
    prev   = &vm->symbols;
    count  = 0;

    while (1) {
        token = COM_Parse(&text_p);
        if (!token[0])
            break;

        segment = ParseHex(token);
        if (segment) {
            COM_Parse(&text_p);
            COM_Parse(&text_p);
            continue; /* only load code segment values */
        }

        token = COM_Parse(&text_p);
        if (!token[0]) {
            Con_Printf(WARNING, "incomplete line at end of file");
            break;
        }
        value = ParseHex(token);

        token = COM_Parse(&text_p);
        if (!token[0]) {
            Con_Printf(WARNING, "incomplete line at end of file");
            break;
        }
        chars = strlen(token);
        sym   = (vmSymbol_t *)Z_Malloc(sizeof(*sym) + chars, TAG_STATIC, &sym, "dbgSymbol");
        *prev = sym;
        
        memset(sym, 0, sizeof(*sym) + chars);
        prev      = &sym->next;
        sym->next = NULL;

        /* convert value from an instruction number to a code offset */
        if (value >= 0 && value < numInstructions)
            value = vm->instructionPointers[value];

        sym->symValue = value;
        N_strncpyz(sym->symName, token, chars + 1);

        count++;
    }

    vm->numSymbols = count;
    Con_Printf("%i symbols parsed from %s", count, symbols);

    Z_Free(mapfile.v);
}

static void VM_StackTrace(vm_t* vm, int programCounter, int programStack)
{
    int count;

    count = 0;
    do {
        Con_Printf("%s", VM_ValueToSymbol(vm, programCounter));
        programStack   = *(int*)&vm->dataBase[programStack + 4];
        programCounter = *(int*)&vm->dataBase[programStack];
    } while (programCounter != -1 && ++count < 32);
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