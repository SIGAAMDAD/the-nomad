#ifndef __MODULE_DEBUGGER__
#define __MODULE_DEBUGGER__

#include <EASTL/map.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include "angelscript/angelscript.h"

class CDebugger
{
public:
	CDebugger( void );
	virtual ~CDebugger();

	// Register callbacks to handle to-string conversions of application types
	// The expandMembersLevel is a counter for how many recursive levels the members should be expanded.
	// If the object that is being converted to a string has members of its own the callback should call
	// the debugger's ToString passing in expandMembersLevel - 1.
	typedef const eastl::string& (*ToStringCallback)( void *obj, int expandMembersLevel, CDebugger *dbg );
	virtual void RegisterToStringCallback( const asITypeInfo *ti, ToStringCallback callback );

	// User interaction
	virtual void Output( const eastl::string& str );

	// Line callback invoked by context
	virtual void LineCallback( asIScriptContext *ctx);

	// Commands
	virtual void PrintHelp( void );
	virtual void AddFileBreakPoint( const eastl::string& file, int32_t lineNbr );
	virtual void AddFuncBreakPoint( const eastl::string& func );
	virtual void ListBreakPoints( void ) const;
	virtual void ListLocalVariables( asIScriptContext *ctx );
	virtual void ListGlobalVariables( asIScriptContext *ctx );
	virtual void ListMemberProperties( asIScriptContext *ctx );
	virtual void ListStatistics( asIScriptContext *ctx );
	virtual void PrintCallstack( asIScriptContext *ctx );
	virtual void PrintValue( const eastl::string& expr, asIScriptContext *ctx );

	// Helpers
	virtual bool CheckBreakPoint( asIScriptContext *ctx );
	virtual const eastl::string& ToString( void *value, asUINT typeId, int32_t expandMembersLevel, asIScriptEngine *engine );

	// Optionally set the engine pointer in the debugger so it can be retrieved
	// by callbacks that need it. This will hold a reference to the engine.
	virtual void SetEngine( asIScriptEngine *engine );
	virtual asIScriptEngine *GetEngine( void );

	friend void Module_DebuggerListGlobalVars_f( void );
	friend void Module_DebuggerPrintHelp_f( void );
	friend void Module_DebuggerListLocalVars_f( void );
	friend void Module_DebuggerListStats_f( void );
	friend void Module_DebuggerListMemberProps_f( void );
	friend void Module_DebuggerPrintValue_f( void );
	friend void Module_SetScriptDebug_f( void );
protected:
	enum DebugAction {
		CONTINUE,  // continue until next break point
		STEP_INTO, // stop at next instruction
		STEP_OVER, // stop at next instruction, skipping called functions
		STEP_OUT   // run until returning from current function
	};
	DebugAction        m_Action;
	asUINT             m_nLastCommandAtStackLevel;
	asIScriptFunction *m_pLastFunction;
	CModuleInfo		  *m_pModule;
	asIScriptEngine	  *m_pEngine;

	struct BreakPoint {
		inline BreakPoint( const eastl::string& f, int32_t n, bool _func )
			: name( f ), lineNbr( n ), func( _func), needsAdjusting( true )
		{
		}

		eastl::string	name;
		int32_t        	lineNbr;
		qboolean        func;
		qboolean        needsAdjusting;
	};
	eastl::vector<BreakPoint> m_BreakPoints;

	// Registered callbacks for converting types to strings
	eastl::map<const asITypeInfo*, ToStringCallback> m_ToStringCallbacks;
};

END_AS_NAMESPACE

#endif