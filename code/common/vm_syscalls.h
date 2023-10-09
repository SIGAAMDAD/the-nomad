#ifndef _VM_SYSCALLS_
#define _VM_SYSCALLS_

#pragma once

typedef intptr_t (*syscallfn_t)(vm_t *, intptr_t *);
typedef struct
{
    intptr_t id;
    syscallfn_t call;
} vmSystemCall_t;

// vm is 32-bit
typedef uint32_t qvm_size_t;
typedef int32_t qvm_ssize_t;
typedef int32_t qvm_intptr_t;
typedef uint32_t qvm_uintptr_t;
typedef int qvm_int_t;

#define OUT_OF_BOUNDS(vm,addr,len) if (VM_MemoryRangeValid(addr,len,vm) != 0) N_Error(ERR_DROP, "%s: out-of-bounds segmentation violation",__func__)

#endif