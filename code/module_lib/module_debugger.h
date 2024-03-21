#ifndef __MODULE_DEBUG__
#define __MODULE_DEBUG__

#pragma once

#include "module_public.h"

class CDebugger
{
public:
	CDebugger( void );
	virtual ~CDebugger();
	
	typedef const char *(* ToStringCallback_t ) ( void *pObject, int32_t nExpandMembersLevel, CDebugger *pDebugger );
	virtual void RegisterToStringCallback( const asITypeInfo *pTypeInfo, ToStringCallback_t pCallback );
	
	virtual void LineCallback( asIScriptContext *pContext );
	
	virtual void AddFileBreakPoint( const char *pFileName, int32_t nLine );
	virtual void AddFuncBreakPoint( const char *pFuncName );
	virtual void ListBreakPoints( void ) const;
	virtual void ListLocalVariables( asIScriptContext *pContext );
	virtual void ListGlobalVariables( asIScriptContext *pContext );
	virtual void ListMemberProperties( asIScriptContext *pContext );
	virtual void PrintCallstack( asIScriptContext *pContext );
	virtual void PrintValue( const char *pExpression, asIScriptContext *pContext );
	
	virtual bool CheckBreakPoint( asIScriptContext *pContext );
	virtual const char *ToString( void *pValue, asUINT nTypeId, int32_t nExpandMembersLevel, asIScriptEngine *pEngine );

	void CmdStepOut( void );
	void CmdContinue( void );
	void PrintHelp( void ) const;
	void CmdStepOver( void );
	void CmdStepInto( void );
	void CmdSetBreakPoint( void );
	void CmdRemoveBreakPoint( void );

	CModuleInfo *m_pModule;
private:
	enum DebugAction {
		CONTINUE,  // continue until breakpoint
		STEP_INTO, // break at next instruction
		STEP_OVER, // break at next instruction, skipping called functions
		STEP_OUT,  // run until return from current function
	};
	DebugAction m_Action;
	asUINT m_nLastCommandAtStackLevel;
	asIScriptFunction *m_pLastFunction;
	
	struct BreakPoint {
		BreakPoint( const char *_pName, int32_t _nLine, bool isFunc )
			: pName( _pName ), nLine( _nLine ), bIsFunc( isFunc ), bNeedsAdjusting( true )
		{
		}
		
		const char *pName;
		int32_t nLine;
		qboolean bIsFunc;
		qboolean bNeedsAdjusting;
	};
	
	UtlVector<BreakPoint> m_BreakPoints;
	UtlHashMap<const asITypeInfo *, ToStringCallback_t> m_ToStringCallbacks;
};

extern CDebugger *g_pDebugger;

#endif