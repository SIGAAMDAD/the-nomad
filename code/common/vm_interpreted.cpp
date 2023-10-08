#include "code/engine/n_shared.h"
#include "vm.h"

#define MAX_OPSTACK_SIZE 512

// macro opcode sequences
typedef enum {
	MOP_LOCAL_LOAD4 = OP_MAX,
	MOP_LOCAL_LOAD4_CONST,
	MOP_LOCAL_LOCAL,
	MOP_LOCAL_LOCAL_LOAD4,
} macro_op_t;

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

int VM_CallInterpreted2( vm_t *vm, uint32_t nargs, int32_t *args )
{
    int32_t	stack[MAX_OPSTACK_SIZE];
	int32_t	*opStack, *opStackTop;
	int32_t	programStack;
	int32_t	stackOnEntry;
	byte	*image;
	int32_t	v1, v0;
	int		dataMask;
	instruction_t *inst, *ci;
	floatint_t	r0, r1;
	int		opcode;
	int32_t	*img;
	int		i;

	// interpret the code
	//vm->currentlyInterpreting = qtrue;

	// we might be called recursively, so this might not be the very top
	programStack = stackOnEntry = vm->programStack;

	// set up the stack frame
	image = vm->dataBase;
	inst = (instruction_t *)vm->codeBase.ptr;
	dataMask = vm->dataMask;

	// leave a free spot at start of stack so
	// that as long as opStack is valid, opStack-1 will
	// not corrupt anything
	opStack = &stack[1];
	opStackTop = stack + arraylen( stack ) - 1;

	programStack -= (MAX_VMMAIN_ARGS + 2) * sizeof( int32_t );
	img = (int*)&image[ programStack ];
	for ( i = 0; i < nargs; i++ ) {
		img[ i + 2 ] = args[ i ];
	}
	img[ 1 ] = 0; 	// return stack
	img[ 0 ] = -1;	// will terminate the loop on return

	ci = inst;

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

	// main interpreter loop, will exit when a LEAVE instruction
	// grabs the -1 program counter

	while ( 1 ) {
		r0.i = opStack[0];
		r1.i = opStack[-1];
    nextInstruction2:
		v0 = ci->value;
		opcode = ci->op;
		ci++;

		switch ( opcode ) {
		case OP_IGNORE:
			ci += v0;
			goto nextInstruction2;

		case OP_BREAK:
			vm->breakCount++;
			goto nextInstruction2;

		case OP_ENTER:
			// get size of stack frame
			programStack -= v0;
			if ( programStack < vm->stackBottom ) {
				VM_Error( vm, "VM programStack overflow" );
			}
			if ( opStack + ((ci-1)->opStack/4) >= opStackTop ) {
				VM_Error( vm, "VM opStack overflow" );
			}
			break;

		case OP_LEAVE:
			// remove our stack frame
			programStack += v0;

			// grab the saved program counter
			v1 = *(int32_t *)&image[ programStack ];
			// check for leaving the VM
			if ( v1 == -1 ) {
				goto done;
			}
            else if ( (unsigned)v1 >= vm->instructionCount ) {
				VM_Error( vm, "VM program counter out of range in OP_LEAVE" );
            }
			ci = inst + v1;
			break;

		case OP_CALL:
			// save current program counter
			*(int *)&image[ programStack ] = ci - inst;

			// jump to the location on the stack
			if ( r0.i < 0 ) {
				// system call
				// save the stack to allow recursive VM entry
				//vm->programStack = programStack - 4;
				vm->programStack = programStack - 8;
				*(int32_t *)&image[ programStack + 4 ] = ~r0.i;
				{
#if __WORDSIZE == 64
					// the vm has ints on the stack, we expect
					// longs so we have to convert it
					intptr_t argarr[16];
					int argn;
					for ( argn = 0; argn < arraylen( argarr ); ++argn ) {
						argarr[ argn ] = *(int32_t*)&image[ programStack + 4 + 4*argn ];
					}
					v0 = vm->systemCall( &argarr[0] );
#else
					v0 = vm->systemCall( (intptr_t *)&image[ programStack + 4 ] );
#endif
				}

				// save return value
				//opStack++;
				ci = inst + *(int32_t *)&image[ programStack ];
				*opStack = v0;
			}
            else if ( r0.u < vm->instructionCount ) {
				// vm call
				ci = inst + r0.i;
				opStack--;
			}
            else {
				VM_Error( vm, "VM program counter out of range in OP_CALL" );
			}
			break;

		// push and pop are only needed for discarded or bad function return values
		case OP_PUSH:
			opStack++;
			break;

		case OP_POP:
			opStack--;
			break;

		case OP_CONST:
			opStack++;
			r1.i = r0.i;
			r0.i = *opStack = v0;
			goto nextInstruction2;

		case OP_LOCAL:
			opStack++;
			r1.i = r0.i;
			r0.i = *opStack = v0 + programStack;
			goto nextInstruction2;

		case OP_JUMP:
			if ( r0.u >= vm->instructionCount ) {
				VM_Error( vm, "VM program counter out of range in OP_JUMP" );
			}
			ci = inst + r0.i;
			opStack--;
			break;

		/*
		===================================================================
		BRANCHES
		===================================================================
		*/

		case OP_EQ:
			opStack -= 2;
			if ( r1.i == r0.i )
				ci = inst + v0;
			break;

		case OP_NE:
			opStack -= 2;
			if ( r1.i != r0.i )
				ci = inst + v0;
			break;

		case OP_LTI:
			opStack -= 2;
			if ( r1.i < r0.i )
				ci = inst + v0;
			break;

		case OP_LEI:
			opStack -= 2;
			if ( r1.i <= r0.i )
				ci = inst + v0;
			break;

		case OP_GTI:
			opStack -= 2;
			if ( r1.i > r0.i )
				ci = inst + v0;
			break;

		case OP_GEI:
			opStack -= 2;
			if ( r1.i >= r0.i )
				ci = inst + v0;
			break;

		case OP_LTU:
			opStack -= 2;
			if ( r1.u < r0.u )
				ci = inst + v0;
			break;

		case OP_LEU:
			opStack -= 2;
			if ( r1.u <= r0.u )
				ci = inst + v0;
			break;

		case OP_GTU:
			opStack -= 2;
			if ( r1.u > r0.u )
				ci = inst + v0;
			break;

		case OP_GEU:
			opStack -= 2;
			if ( r1.u >= r0.u )
				ci = inst + v0;
			break;

		case OP_EQF:
			opStack -= 2;
			if ( r1.f == r0.f )
				ci = inst + v0;
			break;

		case OP_NEF:
			opStack -= 2;
			if ( r1.f != r0.f )
				ci = inst + v0;
			break;

		case OP_LTF:
			opStack -= 2;
			if ( r1.f < r0.f )
				ci = inst + v0;
			break;

		case OP_LEF:
			opStack -= 2;
			if ( r1.f <= r0.f )
				ci = inst + v0;
			break;

		case OP_GTF:
			opStack -= 2;
			if ( r1.f > r0.f )
				ci = inst + v0;
			break;

		case OP_GEF:
			opStack -= 2;
			if ( r1.f >= r0.f )
				ci = inst + v0;
			break;

		//===================================================================

		case OP_LOAD1:
			r0.i = *opStack = image[ r0.i & dataMask ];
			goto nextInstruction2;

		case OP_LOAD2:
			r0.i = *opStack = *(unsigned short *)&image[ r0.i & dataMask ];
			goto nextInstruction2;

		case OP_LOAD4:
			r0.i = *opStack = *(int32_t *)&image[ r0.i & dataMask ];
			goto nextInstruction2;

		case OP_STORE1:
			image[ r1.i & dataMask ] = r0.i;
			opStack -= 2;
			break;

		case OP_STORE2:
			*(short *)&image[ r1.i & dataMask ] = r0.i;
			opStack -= 2;
			break;

		case OP_STORE4:
			*(int *)&image[ r1.i & dataMask ] = r0.i;
			opStack -= 2;
			break;

		case OP_ARG:
			// single byte offset from programStack
			*(int32_t *)&image[ ( v0 + programStack ) /*& ( dataMask & ~3 ) */ ] = r0.i;
			opStack--;
			break;

		case OP_BLOCK_COPY:
			{
				int		*src, *dest;
				int		count, srci, desti;

				count = v0;
				// MrE: copy range check
				srci = r0.i & dataMask;
				desti = r1.i & dataMask;
				count = ((srci + count) & dataMask) - srci;
				count = ((desti + count) & dataMask) - desti;

				src = (int *)&image[ srci ];
				dest = (int *)&image[ desti ];

				memcpy( dest, src, count );
				opStack -= 2;
			}
			break;

		case OP_SEX8:
			*opStack = (signed char)*opStack;
			break;

		case OP_SEX16:
			*opStack = (signed short)*opStack;
			break;

		case OP_NEGI:
			*opStack = -r0.i;
			break;

		case OP_ADD:
			*(--opStack) = r1.i + r0.i;
			break;

		case OP_SUB:
			*(--opStack) = r1.i - r0.i;
			break;

		case OP_DIVI:
			*(--opStack) = r1.i / r0.i;
			break;

		case OP_DIVU:
			*(--opStack) = r1.u / r0.u;
			break;

		case OP_MODI:
			*(--opStack) = r1.i % r0.i;
			break;

		case OP_MODU:
			*(--opStack) = r1.u % r0.u;
			break;

		case OP_MULI:
			*(--opStack) = r1.i * r0.i;
			break;

		case OP_MULU:
			*(--opStack) = r1.u * r0.u;
			break;

		case OP_BAND:
			*(--opStack) = r1.u & r0.u;
			break;

		case OP_BOR:
			*(--opStack) = r1.u | r0.u;
			break;

		case OP_BXOR:
			*(--opStack) = r1.u ^ r0.u;
			break;

		case OP_BCOM:
			*opStack = ~ r0.u;
			break;

		case OP_LSH:
			*(--opStack) = r1.i << r0.i;
			break;

		case OP_RSHI:
			*(--opStack) = r1.i >> r0.i;
			break;

		case OP_RSHU:
			*(--opStack) = r1.u >> r0.i;
			break;

		case OP_NEGF:
			*(float *)opStack =  - r0.f;
			break;

		case OP_ADDF:
			*(float *)(--opStack) = r1.f + r0.f;
			break;

		case OP_SUBF:
			*(float *)(--opStack) = r1.f - r0.f;
			break;

		case OP_DIVF:
			*(float *)(--opStack) = r1.f / r0.f;
			break;

		case OP_MULF:
			*(float *)(--opStack) = r1.f * r0.f;
			break;

		case OP_CVIF:
			*(float *)opStack = (float) r0.i;
			break;

		case OP_CVFI:
			*opStack = (int) r0.f;
			break;

		case MOP_LOCAL_LOAD4:
			ci++;
			opStack++;
			r1.i = r0.i;
			r0.i = *opStack = *(int32_t *)&image[ v0 + programStack ];
			goto nextInstruction2;

		case MOP_LOCAL_LOAD4_CONST:
			r1.i = opStack[1] = *(int32_t *)&image[ v0 + programStack ];
			r0.i = opStack[2] = (ci+1)->value;
			opStack += 2;
			ci += 2;
			goto nextInstruction2;

		case MOP_LOCAL_LOCAL:
			r1.i = opStack[1] = v0 + programStack;
			r0.i = opStack[2] = ci->value + programStack;
			opStack += 2;
			ci++;
			goto nextInstruction2;

		case MOP_LOCAL_LOCAL_LOAD4:
			r1.i = opStack[1] = v0 + programStack;
			r0.i /*= opStack[2]*/ = ci->value + programStack;
			r0.i = opStack[2] = *(int32_t *)&image[ r0.i /*& dataMask*/ ];
			opStack += 2;
			ci += 2;
			goto nextInstruction2;
		}
	}

done:
	//vm->currentlyInterpreting = qfalse;

	if ( opStack != &stack[2] ) {
		N_Error( ERR_FATAL, "Interpreter error: opStack = %ld", (long int) (opStack - stack) );
	}

	vm->programStack = stackOnEntry;

	// return the result
	return *opStack;
}
