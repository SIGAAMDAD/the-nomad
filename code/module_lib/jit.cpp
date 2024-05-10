#include "module_public.h"
#include "asmjit/asmjit.h"

#define JUMP_DESTINATION (void *)(uintptr_t)0x1
#define offset0 (asBC_SWORDARG0( pByteCode ) * sizeof( asDWORD ) )
#define offset1 (asBC_SWORDARG1( pByteCode ) * sizeof( asDWORD ) )
#define offset2 (asBC_SWORDARG2( pByteCode ) * sizeof( asDWORD ) )

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
	asmjit::JitRuntime m_RunTime;
	asmjit::CodeHolder m_CodeHolder;
	asmjit::FileLogger *m_pLogger;
	JitCompiler *m_pCompiler;
	
	FILE *m_pFileHandle;
	uint8_t **m_pActiveJumpTable;
	asUINT m_nCurrentTableSize;
};

CScriptJITCompiler::CScriptJITCompiler( void )
{
	const char *path;
	
	m_CodeHolder.init( m_RunTime.environment(), m_RunTime.cpuFeatures() );
    m_pCompiler = new ( Z_Malloc( sizeof( *m_pCompiler ), TAG_STATIC ) ) JitCompiler( &m_CodeHolder );
	
	path = FS_BuildOSPath( FS_GetHomePath(), NULL, "cache/debugasm.txt" );
	m_pFileHandle = fopen( path, "wb" );
	
	m_pLogger = new ( Z_Malloc( sizeof( *m_pLogger ), TAG_STATIC ) ) asmjit::FileLogger( m_pFileHandle );
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

void CScriptJITCompiler::CompileJITFunction( asDWORD *pByteCode, asDWORD *pEnd, asIScriptFunction *pFunction )
{
	asmjit::FuncFrame funcFrame;
	asmjit::FuncDetail func;
	asmjit::Label label, epilog, jump;
	asmjit::FuncArgsAssignment argMapper;
	Register zax, zdx, zsi, zdi;
	Builder emitter;
	asEBCInstr opCode;
	size_t firstEntry;
	
	Assembler a( &m_CodeHolder );
	
	label = a.newLabel();
	epilog = a.newLabel();
	jump = a.newLabel();
	a.bind( label );
	
	zax = a.zax();
	zdx = a.zdx();
	zsi = a.zsi();
	zdi = a.zdi();
	
	// setup the function
	func.init( asmjit::FuncSignatureT<void, asSVMRegisters *, asPWORD>( asmjit::CallConvId::kCDecl ), m_RunTime.environment() );
	
	funcFrame.init( func );
	funcFrame.setPreservedFP();
	
	argMapper.setFuncDetail( &func );
	argMapper.assignAll( zax, zdx );
	argMapper.updateFuncFrame( funcFrame );
	
	emitter.emitProlog( funcFrame );
	emitter.emitArgsAssignment( funcFrame, argMapper );
	
	// assembly offset
	firstEntry = a.offset();
	
	// initialize FPU
	a.finit();
	
    // asSVMRegister::stackPointer is essentially esi
	while ( pByteCode < pEnd ) {
		opCode = asEBCInstr( *(byte *)pByteCode );
		
		switch ( opCode ) {
		case asBC_PopPtr:
			break;
		case asBC_CpyVtoV4:
            a.mov( a.zbx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
            a.inc( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
            a.inc( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, programPointer ) ) );
            a.mov( a.zbx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
			break;
		case asBC_PshGPtr:
			a.mov( a.zbx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
			a.lea( a.zcx(), ptr( a.zbx(), -AS_PTR_SIZE ) );
			break;
		case asBC_PshC4:
			a.dec( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
			a.mov( a.zbx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
			a.mov( a.zcx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, programPointer ) ) );
			a.mov( dword_ptr( a.zbx() ), a.zcx() );
			a.mov( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ), a.zbx() );
			a.add( a.zcx(), 2 );
			break;
		case asBC_PshC8:
            a.dec( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
			a.mov( a.zbx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
			a.mov( a.zcx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, programPointer ) ) );
			a.mov( qword_ptr( a.zbx() ), a.zcx() );
			a.mov( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ), a.zbx() );
			a.add( a.zcx(), 2 );
			break;
		case asBC_PshNull:
            // mov %ebx [eax+stackPointer]
            // lea %ecx [ebx+$AS_PTR_SIZE]
			a.mov( a.zbx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
            a.lea( a.zcx(), ptr( a.zbx(), -AS_PTR_SIZE ) );
            a.mov( ptr( a.zbx() ), a.zcx() );
            a.mov( native::ptr( a.zbx() ), NULL );
            a.mov( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ), a.zbx() );
            a.inc( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, programPointer ) ) );
			break;
        case asBC_PshV4:
            a.sub( native::dword_ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ), -AS_PTR_SIZE );
            a.mov( a.zbx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
            a.mov( a.zcx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackFramePointer ) ) );
            a.sub( a.zcx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, programPointer ) ) );
            a.mov( ptr( a.zbx() ), a.zcx() );
            a.mov( a.zbx(), ptr( a.zbx() ) );
            a.mov( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ), a.zbx() );
            a.inc( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, programPointer ) ) );
            break;
		case asBC_PshV8:
			a.sub( native::qword_ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ), -AS_PTR_SIZE );
            a.mov( a.zbx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
            a.mov( a.zcx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackFramePointer ) ) );
            a.sub( a.zcx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, programPointer ) ) );
            a.mov( ptr( a.zbx() ), a.zcx() );
            a.mov( a.zbx(), ptr( a.zbx() ) );
            a.mov( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ), a.zbx() );
            a.inc( ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, programPointer ) ) );
			break;
        case asBC_INCi8:
            // mov %ebx [eax+stackPointer]
            // inc %ebx
            // mov %eax %ebx
            a.mov( a.zbx(), native::byte_ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
            a.inc( a.zbx() );
            a.mov( native::byte_ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ), a.zbx() );
            break;
        case asBC_INCi16:
            // mov %ebx [eax+stackPointer]
            // inc %ebx
            // mov %eax %ebx
            a.mov( a.zbx(), native::word_ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
            a.inc( a.zbx() );
            a.mov( native::word_ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ), a.zbx() );
            break;
        case asBC_INCi:
            // mov %ebx [eax+stackPointer]
            // inc %ebx
            // mov %eax %ebx
            a.mov( a.zbx(), native::dword_ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
            a.inc( a.zbx() );
            a.mov( native::dword_ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ), a.zbx() );
			break;
        case asBC_INCi64:
            // mov %ebx [eax+stackPointer]
            // inc %ebx
            // mov %eax %ebx
            a.mov( a.zbx(), native::qword_ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ) );
            a.inc( a.zbx() );
            a.mov( native::qword_ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, stackPointer ) ), a.zbx() );
            break;
		case asBC_SUSPEND:
			a.nop();
			break;
		case asBC_JMP:
			a.jmp( jump );
			break;
		case asBC_RET:
			a.jmp( epilog );
			break;
		case asBC_JNZ:
			a.jnz( jump );
			break;
		case asBC_JS:
			a.js( jump );
			break;
		case asBC_JNS:
			a.jns( jump );
			break;
		case asBC_JNP:
			a.jnp( jump );
			break;
		case asBC_JZ:
			a.jz( jump );
			break;
        case asBC_TYPEID:
            
            break;
		};
	}
	// bind a jump table target
	a.bind( jump );
	a.mov( a.zbx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, programPointer ) ) );
	a.mov( a.zcx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, programPointer ) ) );
	a.add( a.zcx(), 4 );
	a.mov( a.zcx(), ptr( a.zcx() ) );
	a.add( a.zcx(), 2 );
	a.sal( a.zcx(), 2 );
	a.add( a.zbx(), a.zcx() );
	a.mov( a.zcx(), ptr( zax, ASMJIT_OFFSET_OF( asSVMRegisters, programPointer ) ) );
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