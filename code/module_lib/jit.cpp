#include "module_public.h"
#include "asmjit/asmjit.h"

static void OutputJitLog( const asmjit::StringLogger& logger );

class CScriptJITCompiler : public asIJITCompiler
{
public:
    CScriptJITCompiler( void );
	~CScriptJITCompiler();
	int CompileFunction( asIScriptFunction *pFunction, asJITFunction *pOutPut );
    void ReleaseJITFunction( asJITFunction func );
};

CScriptJITCompiler::CScriptJITCompiler( void ) {
}

CScriptJITCompiler::~CScriptJITCompiler() {
}

int CScriptJITCompiler::CompileFunction( asIScriptFunction *pFunction, asJITFunction *pOutPut )
{
    
}
