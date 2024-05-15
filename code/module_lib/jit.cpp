#include "module_public.h"
#include "asmjit/asmjit.h"

#define JUMP_DESTINATION (void *)(uintptr_t)0x1
#define offset0 (asBC_SWORDARG0( pByteCode ) * sizeof( asDWORD ) )
#define offset1 (asBC_SWORDARG1( pByteCode ) * sizeof( asDWORD ) )
#define offset2 (asBC_SWORDARG2( pByteCode ) * sizeof( asDWORD ) )

#define FUNCTION_RESERVE_SPACE 5 * sizeof( void * )

#ifdef _WIN32
#define GDR_STDCALL __stdcall
#else
#define GDR_STDCALL
#endif

static void OutputJitLog( const asmjit::StringLogger& logger );

#if defined(GDRx64)
namespace native = asmjit::x86;
#elif defined(arm64)
namespace native = asmjit::a64;
#endif

typedef native::Compiler JitCompiler;
typedef native::Gp Register;
typedef native::Assembler Assembler;
typedef native::Builder Builder;

class CScriptJITCompiler : public asIJITCompiler
{
public:
	CScriptJITCompiler( void );
	~CScriptJITCompiler();
	int CompileFunction( asIScriptFunction *pFunction, asJITFunction *pOutPut );
	void ReleaseJITFunction( asJITFunction func );

    void SetErrorHandler( asmjit::ErrorHandler *err );
    asmjit::ErrorHandler *GetErrorHandler( void );
    void CompileJITFunction( asDWORD *pByteCode, asDWORD *pEnd, asIScriptFunction *pFunction );
private:
//	void EmitNullPointerException( int nIndex, EVMAbortException iReason );
//	void EmitThrowException( EVMAbortException iReason );
//	asmjit::Label EmitThrowExceptionLabel( EVMAbortException iReason );
//	void ThrowException( int iReason );
	
	asmjit::JitRuntime m_RunTime;
	asmjit::CodeHolder m_CodeHolder;
	asmjit::FileLogger *m_pLogger;
	Assembler m_Assembler;
	JitCompiler *m_pCompiler;
	
	FILE *m_pFileHandle;
	uint8_t **m_pActiveJumpTable;
	asUINT m_nCurrentTableSize;
}

/*
void EmitNullPointerException( int nIndex, EVMAbortException iReason );

void CScriptJITCompiler::ThrowException( int iReason )
{
	ThrowAbortException( (EVMAbortException)iReason, NULL );
}

void CScriptJITCompiler::EmitThrowException( EVMAbortException iReason )
{
	auto call = CreateCall<void, int>( &CScriptJITCompiler::ThrowException );
	call->SetArg( 0, asmjit::imm( iReason ) );
}

asmjit::Label CScriptJITCompiler::EmitThrowExceptionLabel( EVMAbortException iReason )
{
	JitLineInfo info;
	auto label = m_Assembler.newLabel();
	auto cursor = m_Assembler.getCursor();
	
	m_Assembler.bind( label );
	EmitThrowException( iReason );
	m_Assembler.setCursor( cursor );
	
	return label;
}
*/

CScriptJITCompiler::CScriptJITCompiler( void )
{
	const char *path;

	path = FS_BuildOSPath( Cvar_VariableString( "fs_homepath" ), NULL, "_cache/asmjit_debug_log.log" );
	m_pFileHandle = fopen( path, "w" );
	if ( !m_pFileHandle ) {
		Con_Printf( COLOR_YELLOW "WARNING: failed to create asmjit log file\n" );
	} else {
		m_pLogger = new ( S_Malloc( sizeof( *m_pLogger ) ) ) asmjit::FileLogger( m_pFileHandle );
	}

	m_CodeHolder.init( m_RunTime.environment(), m_RunTime.cpuFeatures() );
}

CScriptJITCompiler::~CScriptJITCompiler() {
	m_CodeHolder.resetErrorHandler();
	m_CodeHolder.reset( asmjit::ResetPolicy::kHard );
	
	if ( m_pLogger ) {
		using asmjit::FileLogger;
		m_pLogger->~FileLogger();
		Z_Free( m_pLogger );
		m_pLogger = NULL;
	}
	if ( m_pFileHandle ) {
		fflush( m_pFileHandle );
		fclose( m_pFileHandle );
		m_pFileHandle = NULL;
	}
}

void CScriptJITCompiler::SetErrorHandler( asmjit::ErrorHandler *err )
{
	m_CodeHolder.setErrorHandler( err );
}

static inline native::Gp IntArg( uint8_t index, uint8_t arg )
{
	
}

static inline unsigned FindPushBatchSize( asDWORD *pNextOp, asDWORD *pEndOfByteCode )
{
	unsigned bytes;
	
	bytes = 0;
	
	while ( pNextOp < pEndOfByteCode ) {
		asEBCInstr op = (asEBCInstr)*(byte *)pNextOp;
		switch ( op ) {
		case asBC_PshC4:
		case asBC_PshV4:
		case asBC_PshG4:
		case asBC_TYPEID:
			bytes += sizeof( asDWORD );
			break;
		case asBC_PshV8:
		case asBC_PshC8:
			bytes += sizeof( asQWORD );
			break;
		case asBC_PSF:
		case asBC_PshVPtr:
		case asBC_PshRPtr:
		case asBC_PshNull:
		case asBC_FuncPtr:
		case asBC_OBJTYPE:
		case asBC_PGA:
		case asBC_VAR:
		case asBC_PshGPtr:
			bytes += sizeof( void * );
			break;
		default:
			return bytes;
		};
		pNextOp += asBCTypeSize[ asBCInfo[op].type ];
	}
	return bytes;
}

static void GDR_STDCALL AllocScriptObject( asCObjectType *pType, asCScriptFunction *pConstructor,
	asIScriptEngine *pEngine, asSVMRegisters *pRegisters )
{
	void *mem;
	
	mem = ( (asCScriptEngine *)pEngine )->CallAlloc( pType );
	
}

void CScriptJITCompiler::CompileJITFunction( asDWORD *pByteCode, asDWORD *pEnd, asIScriptFunction *pFunction )
{
	asmjit::FuncFrame funcFrame;
	asmjit::FuncDetail func;
	asmjit::Label label, epilog, jump;
	asmjit::FuncArgsAssignment argMapper;
	Register eax, esi, ebp, edx, ebx, esp, edi, ecx;
	Register rax, rsi, rbp, rdx, rbx, rsp, rdi, rcx;
	Register ax, bx, cx, dx;
	Builder emitter;
	asEBCInstr opCode;
	bool firstJitEntry;
	bool waitingForEntry;
	unsigned reservedPushBytes = 0;
	Assembler a( &m_CodeHolder );
	volatile byte *retPtr;
	size_t firstEntry;
	
	// NOTE: using 32 bit instructions will zero the top of the 64 bit reigsters, but that's only the 32 bit registers, not the 16 bit ones
	// NOTE: mov can only have one argument that is a variable, moving register value to register value is fine
	// NOTE: mov can only move registers of the same size
	// NOTE: 64 bit registers cannot take in a 64 bit immediate value, only a 32 bit value which it will then sign extend
	
	// 64 bit registers
	// rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, r8, r9, r10, r11, r12, r13, r14, r15
	// accessing the other register versions of r8+ are as simple as r[number]D (32 bit), r[number]W (16 bit), r[number]B (8 byte)
	
	// 32 bit registers
	// NOTE: cx is the low 16 bits of 'ecx', and ch & cl are parts of cx
	// general purpose: eax, ebx, ecx, edx
	// index regs: esi, edi, ebp, esi
	// instruction pointer: eip
	
	// 16 bit registers
	// general purpose: ax, bx, cx, dx
	// index regs: si, di, bp, sp
	// instruction pointer ip
	// segment registers: cs, ds, es, ss, fs, gs
	
	// 8 bit reigters
	// general purpose: ah, al, bh, bl, ch, cl, dh, dl
	
	// ip - instruction pointer
	// bp - frame pointer
	// ax - scratch register
	
	eax = native::eax;
	edx = native::edx;
	esi = native::esi;
	ebp = native::ebp;
	ebx = native::ebx;
	esp = native::esp;
	edi = native::edi;
	ecx = native::ecx;
	
	rax = native::rax;
	rdx = native::rdx;
	rsi = native::rsi;
	rbx = native::rbx;
	rsp = native::rsp;
	rsi = native::rsi;
	rcx = native::rcx;
	
	label = a.newLabel();
	epilog = a.newLabel();
	jump = a.newLabel();
	a.bind( label );
	
	// setup the function
	func.init( asmjit::FuncSignatureT<void, asSVMRegisters *, asPWORD>( asmjit::CallConvId::kCDecl ), m_RunTime.environment() );
	
	funcFrame.init( func );
	funcFrame.setPreservedFP();
	
	argMapper.setFuncDetail( &func );
	argMapper.assignAll( rax, rdx );
	argMapper.updateFuncFrame( funcFrame );
	
	emitter.emitProlog( funcFrame );
	emitter.emitArgsAssignment( funcFrame, argMapper );
	
	// initialize FPU
	a.finit();
	
	auto Return = [&]( bool bExpected ) {
		// set rdx to the bytecode pointer so the vm can be returned to the correct state
		a.mov( rdx, (uintptr_t)(void *)pByteCode );
		a.jmp( (uintptr_t)(volatile void *)retPtr );
		waitingForEntry = bExpected;
	};

	firstEntry = a.offset();
	
    // asSVMRegister::stackPointer is essentially esi
	while ( pByteCode < pEnd ) {
		opCode = asEBCInstr( *(byte *)pByteCode );
		
	#define pushPrep( size ) \
		if ( reservedPushBytes == 0 ) { \
			reservedPushBytes = FindPushBatchSize( pByteCode, pEnd ); \
			a.sub( esi, reservedPushBytes ); \
		} \
		reservedPushBytes += size;
		
		switch ( opCode ) {
		default:
			// undefined opCode, force a debug break
			a.int3();
			break;
		case asBC_JitEntry: {
			if ( !firstJitEntry ) {
				firstJitEntry = (void *)pByteCode;
			}
			break; }
		case asBC_SUSPEND:
			a.nop();
			break;
		case asBC_PshVPtr:
			pushPrep( sizeof( void * ) );
			a.mov( rax, ptr( edi, -offset0 ) );
			a.mov( ptr( edi, reservedPushBytes ), rax );
			break;
		case asBC_PshRPtr:
			pushPrep( sizeof( void * ) );
			a.mov( rax, ptr( edi, -offset0 ) );
			a.mov( ptr( esi, reservedPushBytes ), rax );
			break;
		case asBC_PshGPtr:
			break;
		case asBC_PopPtr:
			a.mov( rbx, ptr( rax, offsetof( asSVMRegisters, stackPointer ) ) );
			a.lea( rcx, ptr( rbx, AS_PTR_SIZE ) );
			a.mov( ptr( rax, offsetof( asSVMRegisters, stackPointer ) ), rcx );
			a.inc( ptr( rax, offsetof( asSVMRegisters, programPointer ) ) );
			break;
		case asBC_PSF:
			pushPrep( sizeof( void * ) );
			a.mov( rax, ptr( edi, -offset0 ) );
			a.mov( ptr( esi, reservedPushBytes ), rax );
			break;
		case asBC_PshV4:
			pushPrep( sizeof( asDWORD ) );
			a.mov( ebx, ptr( eax, offsetof( asSVMRegisters, stackPointer ) ) );
			a.mov( ecx, ptr( eax, offsetof( asSVMRegisters, stackFramePointer ) ) );
			a.sub( ecx, ptr( eax, offsetof( asSVMRegisters, programPointer ) ) );
			a.mov( ebx, ptr( ecx ) );
			a.mov( ebx, ptr( ebx ) );
			a.mov( ptr( eax, offsetof( asSVMRegisters, stackPointer ) ), ebx );
			a.inc( ptr( eax, offsetof( asSVMRegisters, programPointer ) ) );
			break;
		case asBC_PshV8:
			pushPrep( sizeof( asQWORD ) );
			a.mov( ebx, ptr( eax, offsetof( asSVMRegisters, stackPointer ) ) );
			a.mov( ecx, ptr( eax, offsetof( asSVMRegisters, stackFramePointer ) ) );
			a.sub( ecx, ptr( eax, offsetof( asSVMRegisters, programPointer ) ) );
			a.mov( ebx, ptr( ecx ) );
			a.mov( ebx, ptr( ebx ) );
			a.mov( ptr( eax, offsetof( asSVMRegisters, stackPointer ) ), ebx );
			a.inc( ptr( eax, offsetof( asSVMRegisters, programPointer ) ) );
			break;
		case asBC_PshNull:
			a.mov( rbx, ptr( rax, offsetof( asSVMRegisters, stackPointer ) ) );
            a.lea( rcx, ptr( rbx, -AS_PTR_SIZE ) );
            a.mov( ptr( rbx ), rcx );
            a.mov( native::qword_ptr( rbx ), NULL );
            a.mov( ptr( eax, offsetof( asSVMRegisters, stackPointer ) ), rbx );
            a.inc( ptr( eax, offsetof( asSVMRegisters, programPointer ) ) );
            break;
		case asBC_PshC4:
			pushPrep( sizeof( asDWORD ) );
			a.mov( ebx, ptr( eax, offsetof( asSVMRegisters, stackPointer ) ) );
			a.mov( ecx, ptr( eax, offsetof( asSVMRegisters, programPointer ) ) );
			a.mov( dword_ptr( ebx ), ecx );
			a.mov( ptr( eax, offsetof( asSVMRegisters, stackPointer ) ), ebx );
			a.add( ecx, 2 );
			break;
		case asBC_PshC8:
			pushPrep( sizeof( asQWORD ) );
			a.mov( ebx, ptr( eax, offsetof( asSVMRegisters, stackPointer ) ) );
			a.mov( ecx, ptr( eax, offsetof( asSVMRegisters, programPointer ) ) );
			a.mov( qword_ptr( ebx ), ecx );
			a.mov( ptr( eax, offsetof( asSVMRegisters, stackPointer ) ), ebx );
			a.add( ecx, 2 );
			break;
		};
	}
	// bind a jump table target
	a.bind( jump );
	a.mov( rbx, ptr( rax, offsetof( asSVMRegisters, programPointer ) ) );
	a.mov( rcx, ptr( rax, offsetof( asSVMRegisters, programPointer ) ) );
	a.add( rcx, 4 );
	a.mov( rcx, ptr( rcx ) );
	a.add( rcx, 2 );
	a.sal( rcx, 2 );
	a.add( rbx, rcx );
	a.mov( rcx, ptr( rax, offsetof( asSVMRegisters, programPointer ) ) );
	a.mov( ptr( a.zcx() ), a.zbx() );
	a.bind( epilog );
	a.nop();
	
	emitter.emitEpilog( funcFrame );
}

typedef int (*JitEntryPoint_t)( asSVMRegisters *, asPWORD );

int CScriptJITCompiler::CompileFunction( asIScriptFunction *pFunction, asJITFunction *pOutPut )
{
	asUINT nLength;
	asDWORD *pByteCode, *pEnd;
	asmjit::Error err;
	
	pByteCode = pFunction->GetByteCode( &nLength );
	
	if ( !pByteCode || !nLength ) {
		return asSUCCESS;
	}
	
	pEnd = pByteCode + nLength;
	
    CompileJITFunction( pByteCode, pEnd, pFunction );
	
	err = m_RunTime.add( pOutPut, &m_CodeHolder );
	if ( err ) {
		return asOUT_OF_MEMORY;
	}
	
	return 0;
}

void CScriptJITCompiler::ReleaseJITFunction( asJITFunction func )
{
	m_RunTime.release( func );
}