#include "module_public.h"
#include "debugger.h"

static CDebugger *s_pDebugger;

void Module_DebuggerListGlobalVars_f( void ) {
	if ( !s_pDebugger->m_pModule ) {
		Con_Printf( COLOR_YELLOW "WARNING: module_lib debug command called, but no script debug is active.\n" );
		return;
	}
	s_pDebugger->ListGlobalVariables( s_pDebugger->m_pModule->m_pHandle->GetContext() );
}

void Module_SetScriptDebug_f( void ) {
	const char *moduleName;

	if ( Cmd_Argc() < 2 ) {
		Con_Printf( "usage: ml_debug.set_script_debug <module name>\n" );
		return;
	}
	moduleName = Cmd_Argv( 1 );
	if ( !N_stricmp( moduleName, "none" ) ) {
		Con_Printf( "ending module debug session.\n" );
		s_pDebugger->m_pModule = NULL;
		return;
	}

	Con_Printf( "beginning debug session for '%s'\n", moduleName );

	s_pDebugger->m_pModule = g_pModuleLib->GetModule( moduleName );
}

void Module_DebuggerListLocalVars_f( void ) {
	if ( !s_pDebugger->m_pModule ) {
		Con_Printf( COLOR_YELLOW "WARNING: module_lib debug command called, but no script debug is active.\n" );
		return;
	}
	s_pDebugger->ListLocalVariables( s_pDebugger->m_pModule->m_pHandle->GetContext() );
}

void Module_DebuggerListStats_f( void ) {
	if ( !s_pDebugger->m_pModule ) {
		Con_Printf( COLOR_YELLOW "WARNING: module_lib debug command called, but no script debug is active.\n" );
		return;
	}
	s_pDebugger->ListStatistics( s_pDebugger->m_pModule->m_pHandle->GetContext() );
}

void Module_DebuggerListMemberProps_f( void ) {
	if ( !s_pDebugger->m_pModule ) {
		Con_Printf( COLOR_YELLOW "WARNING: module_lib debug command called, but no script debug is active.\n" );
		return;
	}
	s_pDebugger->ListMemberProperties( s_pDebugger->m_pModule->m_pHandle->GetContext() );
}

void Module_DebuggerPrintValue_f( void ) {
	if ( !s_pDebugger->m_pModule ) {
		Con_Printf( COLOR_YELLOW "WARNING: module_lib debug command called, but no script debug is active.\n" );
		return;
	}
	if ( Cmd_Argc() < 2 ) {
		Con_Printf( "usage: ml_debug.print_value <value name>\n" );
		return;
	}

	s_pDebugger->PrintValue( Cmd_Argv( 1 ), s_pDebugger->m_pModule->m_pHandle->GetContext() );
}

void Module_DebuggerPrintHelp_f( void ) {
	s_pDebugger->PrintHelp();
}

CDebugger::CDebugger( void ) {
	m_Action = CONTINUE;
	m_pLastFunction = NULL;
	s_pDebugger = this;
	m_pEngine = g_pModuleLib->GetScriptEngine();

	Cmd_AddCommand( "ml_debug.set_script_debug", Module_SetScriptDebug_f );
	Cmd_AddCommand( "ml_debug.list_global_vars", Module_DebuggerListGlobalVars_f );
	Cmd_AddCommand( "ml_debug.list_local_vars", Module_DebuggerListLocalVars_f );
	Cmd_AddCommand( "ml_debug.print_value", Module_DebuggerPrintValue_f );
	Cmd_AddCommand( "ml_debug.print_help", Module_DebuggerPrintHelp_f );
}

CDebugger::~CDebugger() {
	SetEngine( NULL );

	Cmd_RemoveCommand( "ml_debug.set_script_debug" );
	Cmd_RemoveCommand( "ml_debug.list_global_vars" );
	Cmd_RemoveCommand( "ml_debug.list_local_vars" );
	Cmd_RemoveCommand( "ml_debug.print_value" );
	Cmd_RemoveCommand( "ml_debug.print_help" );
}

const eastl::string& CDebugger::ToString( void *value, asUINT typeId, int32_t expandMembers, asIScriptEngine *engine )
{
	static eastl::string str;

	if ( !value ) {
		return "<null>";
	}

	// If no engine pointer was provided use the default
	if ( !engine ) {
		engine = m_pEngine;
	}

	switch ( typeId ) {
	case asTYPEID_VOID: return "<void>";
	case asTYPEID_BOOL:
		return str.sprintf( "%s", *(bool *)value ? "true" : "false" );
	case asTYPEID_INT8:
		str.sprintf( "%h", *(signed char *)value );
		break;
	case asTYPEID_INT16:
		str.sprintf( "%h", *(signed short *)value );
		break;
	case asTYPEID_INT32:
		str.sprintf( "%i", *(signed int *)value );
		break;
	case asTYPEID_INT64:
		str.sprintf( "%li", *(asINT64 *)value );
		break;
	case asTYPEID_UINT8:
		str.sprintf( "%hu", *(unsigned char *)value );
		break;
	case asTYPEID_UINT16:
		str.sprintf( "%hu", *(unsigned short *)value );
		break;
	case asTYPEID_UINT32:
		str.sprintf( "%u", *(unsigned int *)value );
		break;
	case asTYPEID_UINT64:
		str.sprintf( "%lu", *(asQWORD *)value );
		break;
	case asTYPEID_FLOAT:
		str.sprintf( "%f", *(float *)value );
		break;
	case asTYPEID_DOUBLE:
		str.sprintf( "%lf", *(double *)value );
		break;
	case asTYPEID_MASK_OBJECT: {
		// the type is an enum
		str.sprintf( "%u", *(asUINT *)value );

		if ( engine ) {
			asITypeInfo *type = engine->GetTypeInfoById( typeId );
			for ( uint32_t n = type->GetEnumValueCount(); n-- > 0; ) {
				int32_t enumVal;
				const char *enumName;

				enumName = type->GetEnumValueByIndex( n, &enumVal );
				if ( enumVal == *(int32_t *)value ) {
					str.append_sprintf( ", %s", enumName );
					break;
				}
			}
		}
		break; }
	case asTYPEID_SCRIPTOBJECT: {
		if ( typeId & asTYPEID_OBJHANDLE ) {
			value = *(void **)value;
		}

		asIScriptObject *obj = (asIScriptObject *)value;

		str.sprintf( "{%p}", (void *)obj );

		if ( obj && expandMembers ) {
			asITypeInfo *type = obj->GetObjectType();
			for ( uint32_t n = 0; n < obj->GetPropertyCount(); n++ ) {
				if ( n == 0 ) {
					str.append( " " );
				} else {
					str.append( ", " );
				}

				str.append_sprintf( "%s = %s",
					type->GetPropertyDeclaration( n ), ToString( obj->GetAddressOfProperty( n ),
					obj->GetPropertyTypeId( n ), expandMembers - 1, type->GetEngine() ).c_str() );
			}
		}
		break; }
	default: {
		if ( typeId & asTYPEID_OBJHANDLE ) {
			value = *(void **)value;
		}

		if ( engine ) {
			asITypeInfo *type = engine->GetTypeInfoById( typeId );
			if ( type->GetFlags() & asOBJ_REF ) {
				str.sprintf( "{%p}", value );
			}
			if ( value ) {
				auto it = m_ToStringCallbacks.find( type );
				if ( it == m_ToStringCallbacks.end() ) {
					if ( type->GetFlags() & asOBJ_TEMPLATE ) {
						asITypeInfo *tmplType = engine->GetTypeInfoByName( type->GetName() );
						it = m_ToStringCallbacks.find( tmplType );
					}
				}

				if ( it != m_ToStringCallbacks.end() ) {
					if ( type->GetFlags() & asOBJ_REF ) {
						str.append( " " );
					}

					str.append_sprintf( "%s", it->second( value, expandMembers, this ).c_str() );
				}
			}
		} else {
			str.append( "{no engine}" );
		}
		break; }
	};
	return str;
}

void CDebugger::RegisterToStringCallback( const asITypeInfo *ot, ToStringCallback callback )
{
	if ( m_ToStringCallbacks.find( ot ) == m_ToStringCallbacks.end() ) {
		m_ToStringCallbacks.insert( eastl::map<const asITypeInfo*, ToStringCallback>::value_type( ot, callback ) );
	}
}

void CDebugger::LineCallback( asIScriptContext *ctx )
{
	const char *file;
	int32_t lineNbr;

	assert( ctx );

	// This should never happen, but it doesn't hurt to validate it
	if ( !ctx ) {
		return;
	}

	// By default we ignore callbacks when the context is not active.
	// An application might override this to for example disconnect the
	// debugger as the execution finished.
	if ( ctx->GetState() != asEXECUTION_ACTIVE ) {
		return;
	}

	if ( m_Action == CONTINUE ) {
		if ( !CheckBreakPoint( ctx ) ) {
			return;
		}
	}
	else if ( m_Action == STEP_OVER ) {
		if ( ctx->GetCallstackSize() > m_nLastCommandAtStackLevel ) {
			if ( !CheckBreakPoint( ctx ) ) {
				return;
			}
		}
	}
	else if ( m_Action == STEP_OUT ) {
		if ( ctx->GetCallstackSize() >= m_nLastCommandAtStackLevel ) {
			if ( !CheckBreakPoint( ctx ) ) {
				return;
			}
		}
	}
	else if ( m_Action == STEP_INTO ) {
		CheckBreakPoint( ctx );

		// Always break, but we call the check break point anyway 
		// to tell user when break point has been reached
	}

	lineNbr = ctx->GetLineNumber( 0, 0, &file );
	Con_Printf( "%s:%i; %s\n",
		( file ? file : "{unnamed}" ), lineNbr, ctx->GetFunction()->GetDeclaration() );
}

bool CDebugger::CheckBreakPoint( asIScriptContext *ctx )
{
	const char *tmp;
	int32_t lineNbr;
	size_t r, n;
	asIScriptFunction *func;
	eastl::string file;

	if ( ctx == 0 ) {
		return false;
	}

	// TODO: Should cache the break points in a function by checking which possible break points
	//       can be hit when entering a function. If there are no break points in the current function
	//       then there is no need to check every line.

	lineNbr = ctx->GetLineNumber( 0, 0, &tmp );

	// Consider just filename, not the full path
	file = tmp ? tmp : "";
	r = file.find_last_of( "\\/" );
	if ( r != eastl::string::npos ) {
		file = file.substr( r + 1 );
	}

	// Did we move into a new function?
	func = ctx->GetFunction();
	if ( m_pLastFunction != func ) {
		// Check if any breakpoints need adjusting
		for ( n = 0; n < m_BreakPoints.size(); n++ ) {
			// We need to check for a breakpoint at entering the function
			if ( m_BreakPoints[n].func ) {
				if ( m_BreakPoints[n].name == func->GetName() ) {
					Con_Printf( "Entering function '%s'. Transforming it into breakpoint\n",
						m_BreakPoints[n].name.c_str() );

					// Transform the function breakpoint into a file breakpoint
					m_BreakPoints[n].name           = file;
					m_BreakPoints[n].lineNbr        = lineNbr;
					m_BreakPoints[n].func           = false;
					m_BreakPoints[n].needsAdjusting = false;
				}
			}
			// Check if a given breakpoint fall on a line with code or else adjust it to the next line
			else if ( m_BreakPoints[n].needsAdjusting &&
					 m_BreakPoints[n].name == file )
			{
				const int32_t line = func->FindNextLineWithCode( m_BreakPoints[n].lineNbr );
				if ( line >= 0 ) {
					m_BreakPoints[n].needsAdjusting = false;
					if ( line != m_BreakPoints[n].lineNbr ) {
						Con_Printf( "Moving breakpoint %lu in file '%s' to next line with code at line %i\n",
							n, file.c_str(), line );

						// Move the breakpoint to the next line
						m_BreakPoints[n].lineNbr = line;
					}
				}
			}
		}
	}
	m_pLastFunction = func;

	// Determine if there is a breakpoint at the current line
	for ( n = 0; n < m_BreakPoints.size(); n++ ) {
		// TODO: do case-less comparison for file name

		// Should we break?
		if ( !m_BreakPoints[n].func &&
			m_BreakPoints[n].lineNbr == lineNbr &&
			m_BreakPoints[n].name == file )
		{
			Con_Printf( "Reached break point %lu in file '%s' at line %i", n, file.c_str(), lineNbr );
			return true;
		}
	}

	return false;
}

void CDebugger::PrintValue(const eastl::string &expr, asIScriptContext *ctx)
{
	if( ctx == 0 )
	{
		Output("No script is running\n");
		return;
	}

	asIScriptEngine *engine = ctx->GetEngine();

	// Tokenize the input eastl::string to get the variable scope and name
	asUINT len = 0;
	eastl::string scope;
	eastl::string name;
	eastl::string str = expr;
	asETokenClass t = engine->ParseToken(str.c_str(), 0, &len);
	while( t == asTC_IDENTIFIER || (t == asTC_KEYWORD && len == 2 && str.compare(0, 2, "::") == 0) )
	{
		if( t == asTC_KEYWORD )
		{
			if( scope == "" && name == "" )
				scope = "::";			// global scope
			else if( scope == "::" || scope == "" )
				scope = name;			// namespace
			else
				scope += "::" + name;	// nested namespace
			name = "";
		}
		else if( t == asTC_IDENTIFIER )
			name.assign(str.c_str(), len);

		// Skip the parsed token and get the next one
		str = str.substr(len);
		t = engine->ParseToken(str.c_str(), 0, &len);
	}

	if( name.size() )
	{
		// Find the variable
		void *ptr = 0;
		int typeId = 0;

		asIScriptFunction *func = ctx->GetFunction();
		if( !func ) return;

		// skip local variables if a scope was informed
		if( scope == "" )
		{
			// We start from the end, in case the same name is reused in different scopes
			for( asUINT n = func->GetVarCount(); n-- > 0; )
			{
				const char* varName = 0;
				ctx->GetVar(n, 0, &varName, &typeId);
				if( ctx->IsVarInScope(n) && varName != 0 && name == varName )
				{
					ptr = ctx->GetAddressOfVar(n);
					break;
				}
			}

			// Look for class members, if we're in a class method
			if( !ptr && func->GetObjectType() )
			{
				if( name == "this" )
				{
					ptr = ctx->GetThisPointer();
					typeId = ctx->GetThisTypeId();
				}
				else
				{
					asITypeInfo *type = engine->GetTypeInfoById(ctx->GetThisTypeId());
					for( asUINT n = 0; n < type->GetPropertyCount(); n++ )
					{
						const char *propName = 0;
						int offset = 0;
						bool isReference = 0;
						int compositeOffset = 0;
						bool isCompositeIndirect = false;
						type->GetProperty(n, &propName, &typeId, 0, 0, &offset, &isReference, 0, &compositeOffset, &isCompositeIndirect);
						if( name == propName )
						{
							ptr = (void*)(((asBYTE*)ctx->GetThisPointer())+compositeOffset);
							if (isCompositeIndirect) ptr = *(void**)ptr;
							ptr = (void*)(((asBYTE*)ptr) + offset);
							if( isReference ) ptr = *(void**)ptr;
							break;
						}
					}
				}
			}
		}

		// Look for global variables
		if( !ptr )
		{
			if( scope == "" )
			{
				// If no explicit scope was informed then use the namespace of the current function by default
				scope = func->GetNamespace();
			}
			else if( scope == "::" )
			{
				// The global namespace will be empty
				scope = "";
			}

			asIScriptModule *mod = func->GetModule();
			if( mod )
			{
				for( asUINT n = 0; n < mod->GetGlobalVarCount(); n++ )
				{
					const char *varName = 0, *nameSpace = 0;
					mod->GetGlobalVar(n, &varName, &nameSpace, &typeId);

					// Check if both name and namespace match
					if( name == varName && scope == nameSpace )
					{
						ptr = mod->GetAddressOfGlobalVar(n);
						break;
					}
				}
			}
		}

		if( ptr )
		{
			// TODO: If there is a . after the identifier, check for members
			// TODO: If there is a [ after the identifier try to call the 'opIndex(expr) const' method 
			if( str != "" )
			{
				Output("Invalid expression. Expression doesn't end after symbol\n");
			}
			else
			{
				// TODO: Allow user to set if members should be expanded
				// Expand members by default to 3 recursive levels only
				Con_Printf( "%s\n", ToString( ptr, typeId, 3, engine ).c_str() );
			}
		}
		else
		{
			Output("Invalid expression. No matching symbol\n");
		}
	}
	else
	{
		Output("Invalid expression. Expected identifier\n");
	}
}

void CDebugger::ListBreakPoints( void ) const
{
	size_t b;

	// List all break points
	for ( b = 0; b < m_BreakPoints.size(); b++ ) {
		if ( m_BreakPoints[b].func ) {
			Con_Printf( "%lu - %s\n", b, m_BreakPoints[b].name.c_str() );
		} else {
			Con_Printf( "%lu - %s:%i\n", b, m_BreakPoints[b].name.c_str(), m_BreakPoints[b].lineNbr );
		}
	}
}

void CDebugger::ListMemberProperties( asIScriptContext *ctx )
{
	void *ptr;

	if ( ctx == 0 ) {
		Output( "No script is running\n" );
		return;
	}

	ptr = ctx->GetThisPointer();
	if ( ptr ) {
		// TODO: Allow user to define if members should be expanded or not
		// Expand members by default to 3 recursive levels only
		Con_Printf( "this = %s\n", ToString( ptr, ctx->GetThisTypeId(), 3, ctx->GetEngine() ).c_str() );
	}
}

void CDebugger::ListLocalVariables( asIScriptContext *ctx )
{
	int typeId;
	uint32_t n;
	const char *name;
	asIScriptFunction *func;

	if ( ctx == 0 ) {
		Output( "No script is running\n" );
		return;
	}

	func = ctx->GetFunction();
	if ( !func ) {
		return;
	}

	for (  n = 0; n < func->GetVarCount(); n++ ) {
		// Skip temporary variables
		// TODO: Should there be an option to view temporary variables too?
		func->GetVar( n, &name );
		if ( name == 0 || strlen( name ) == 0 ) {
			continue;
		}

		if ( ctx->IsVarInScope( n ) ) {
			// TODO: Allow user to set if members should be expanded or not
			// Expand members by default to 3 recursive levels only
			ctx->GetVar( n, 0, 0, &typeId );
			Con_Printf( "%s = %s\n",
				func->GetVarDecl( n ), ToString( ctx->GetAddressOfVar( n ), typeId, 3, ctx->GetEngine() ).c_str() );
		}
	}
}

void CDebugger::ListGlobalVariables( asIScriptContext *ctx )
{
	int typeId;
	uint32_t n;

	if ( ctx == 0 ) {
		Output( "No script is running\n" );
		return;
	}

	// Determine the current module from the function
	asIScriptFunction *func = ctx->GetFunction();
	if ( !func ) {
		return;
	}

	asIScriptModule *mod = func->GetModule();
	if ( !mod ) {
		return;
	}

	for ( n = 0; n < mod->GetGlobalVarCount(); n++ ) {
		mod->GetGlobalVar( n, 0, 0, &typeId );
		// TODO: Allow user to set how many recursive expansions should be done
		// Expand members by default to 3 recursive levels only
		Con_Printf( "%s = %s\n",
			mod->GetGlobalVarDeclaration( n ), ToString( mod->GetAddressOfGlobalVar( n ), typeId, 3, ctx->GetEngine() ).c_str() );
	}
}

void CDebugger::ListStatistics( asIScriptContext *ctx )
{
	if ( ctx == 0 ) {
		Output( "No script is running\n" );
		return;
	}

	const asIScriptEngine *engine = ctx->GetEngine();
	uint32_t gcCurrSize, gcTotalDestr, gcTotalDet, gcNewObjects, gcTotalNewDestr;

	engine->GetGCStatistics( &gcCurrSize, &gcTotalDestr, &gcTotalDet, &gcNewObjects, &gcTotalNewDestr );

	Con_Printf(
		"Garbage Collector:\n"
		" current size:         %-8u\n"
		" total freed:          %-8u\n"
		" total detected:       %-8u\n"
		" new objects:          %-8u\n"
		" new objects destroyed:%-8u\n"
	, gcCurrSize, gcTotalDestr, gcTotalDet, gcNewObjects, gcTotalNewDestr );
}

void CDebugger::PrintCallstack( asIScriptContext *ctx )
{
	const char *file;
	int32_t lineNbr;
	uint32_t n;

	if ( !ctx ) {
		Output( "No script is running\n" );
		return;
	}

	lineNbr = 0;

	for ( n = 0; n < ctx->GetCallstackSize(); n++ ) {
		lineNbr = ctx->GetLineNumber( n, 0, &file );
		Con_Printf( "%s:%i; %s\n",
			( file ? file : "{unnamed}" ), lineNbr, ctx->GetFunction( n )->GetDeclaration() );
	}
}

void CDebugger::AddFuncBreakPoint( const eastl::string& func )
{
	size_t b, e;

	// Trim the function name
	b = func.find_first_not_of( " \t" );
	e = func.find_last_not_of( " \t"  );
	eastl::string&& actual = eastl::move( func.substr( b, e != eastl::string::npos ? e - b + 1 : eastl::string::npos ) );

	Con_Printf( "Adding deferred break point for function '%s'\n", actual.c_str() );

	m_BreakPoints.push_back( BreakPoint( actual, 0, true ) );
}

void CDebugger::AddFileBreakPoint( const eastl::string& file, int32_t lineNbr )
{
	size_t r, b, e;
	eastl::string actual;

	// Store just file name, not entire path
	r = file.find_last_of( "\\/" );
	if ( r != eastl::string::npos ) {
		actual = file.substr( r + 1 );
	} else {
		actual = file;
	}

	// Trim the file name
	b = actual.find_first_not_of( " \t" );
	e = actual.find_last_not_of( " \t" );
	actual = actual.substr( b, e != eastl::string::npos ? e - b + 1 : eastl::string::npos );

	Con_Printf( "Setting break point in file '%s' at line %i\n", actual.c_str(), lineNbr );

	m_BreakPoints.push_back( BreakPoint( actual, lineNbr, false ) );
}

void CDebugger::PrintHelp( void )
{
//	Con_Printf(
//		"debugging commands for vm:\n"
//		"br      set a debug breakpoint at specified location\n"
//		"bt      print a stacktrace\n"
//		"p       print the value of specified variable or function\n"
//		"cont    continue execution\n"
//		"n       execute the next line of code");

	Con_Printf(
	//	" c - Continue\n"
	//    " s - Step into\n"
	//    " n - Next step\n"
	//    " o - Step out\n"
	//    " b - Set break point\n"
	//	" r - Remove break point\n"
	    " ml_debug.list_global_vars - list global module variables\n"
		" ml_debug.list_local_vars - list local module variables\n"
		" ml_debug.list_member_properties - list module member properties\n"
		" ml_debug.list_stats - list module memory statistics\n"
	    " ml_debug.print_value - print value\n"
//	    " w - Where am I?\n"
//	    " a - abort module execution\n"
	    " ml_debug.print_help - print this help text\n" );
}

void CDebugger::Output( const eastl::string& str ) {
	Con_Printf( "%s", str.c_str() );
}

void CDebugger::SetEngine( asIScriptEngine *engine )
{
	if ( m_pEngine != engine ) {
		if ( m_pEngine ) {
			m_pEngine->Release();
		}
		m_pEngine = engine;
		if ( m_pEngine ) {
			m_pEngine->AddRef();
		}
	}
}

asIScriptEngine *CDebugger::GetEngine( void ) {
	return m_pEngine;
}
