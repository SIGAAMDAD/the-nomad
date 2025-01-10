#include "../module_public.h"
#include "module_funcdefs.h"

void CvarRegister( const string_t *name, string_t *value, asDWORD flags, asINT32 *intValue, float *floatValue,
	asINT32 *modificationCount, cvarHandle_t *cvarHandle )
{
	vmCvar_t vmCvar;

	Cvar_Register( &vmCvar, name->c_str(), value->c_str(), flags, CVAR_PRIVATE );

	*value = (const char *)vmCvar.s;
	*intValue = vmCvar.i;
	*floatValue = vmCvar.f;
	*modificationCount = vmCvar.modificationCount;
	*cvarHandle = vmCvar.handle;
}
void CvarUpdate( string_t *value, asINT32 *intValue, float *floatValue, asINT32 *modificationCount, cvarHandle_t cvarHandle )
{
	vmCvar_t vmCvar;

	memset( &vmCvar, 0, sizeof( vmCvar ) );
	vmCvar.handle = cvarHandle;
	vmCvar.modificationCount = *modificationCount;
	Cvar_Update( &vmCvar, CVAR_PRIVATE );

	*value = (const char *)vmCvar.s;
	*intValue = vmCvar.i;
	*floatValue = vmCvar.f;
	*modificationCount = vmCvar.modificationCount;
}
int CvarVariableInteger( const string_t *name )
{ return Cvar_VariableInteger( name->c_str() ); }
float CvarVariableFloat( const string_t *name )
{ return Cvar_VariableFloat( name->c_str() ); }
const string_t CvarVariableString( const string_t *name )
{ return Cvar_VariableString( name->c_str() ); }
void CvarSet( const string_t *name, const string_t *value )
{ Cvar_Set( name->c_str(), value->c_str() ); }

const string_t CmdArgv( asDWORD nArg )
{ return Cmd_Argv( nArg ); }
const string_t CmdArgs( asDWORD nArg )
{ return Cmd_ArgsFrom( nArg ); }
void CmdAddCommand( const string_t *name )
{ Cmd_AddCommand( name->c_str(), NULL ); }
void CmdRemoveCommand( const string_t *name )
{ Cmd_RemoveCommand( name->c_str() ); }
void CmdExecuteCommand( const string_t *cmd )
{ Cbuf_ExecuteText( EXEC_APPEND, cmd->c_str() ); }

bool IsAnyKeyDown( void )
{ return Key_AnyDown(); }
bool IsKeyDown( uint32_t nKeyNum )
{ return Key_IsDown( nKeyNum ); }
uint32_t KeyGetKey( const string_t *str )
{ return Key_GetKey( str->c_str() ); }
const string_t KeyGetBinding( uint32_t nKeyNum )
{ return Key_GetBinding( nKeyNum ); }

void ScriptLib_Register_Engine( void )
{
	SET_NAMESPACE( "TheNomad::Engine" );
	{
		REGISTER_ENUM_TYPE( "KeyNum" );
		REGISTER_ENUM_VALUE( "KeyNum", "A", KEY_A );
		REGISTER_ENUM_VALUE( "KeyNum", "B", KEY_B );
		REGISTER_ENUM_VALUE( "KeyNum", "C", KEY_C );
		REGISTER_ENUM_VALUE( "KeyNum", "D", KEY_D );
		REGISTER_ENUM_VALUE( "KeyNum", "E", KEY_E );
		REGISTER_ENUM_VALUE( "KeyNum", "F", KEY_F );
		REGISTER_ENUM_VALUE( "KeyNum", "G", KEY_G );
		REGISTER_ENUM_VALUE( "KeyNum", "H", KEY_H );
		REGISTER_ENUM_VALUE( "KeyNum", "I", KEY_I );
		REGISTER_ENUM_VALUE( "KeyNum", "J", KEY_J );
		REGISTER_ENUM_VALUE( "KeyNum", "K", KEY_K );
		REGISTER_ENUM_VALUE( "KeyNum", "L", KEY_L );
		REGISTER_ENUM_VALUE( "KeyNum", "M", KEY_M );
		REGISTER_ENUM_VALUE( "KeyNum", "N", KEY_N );
		REGISTER_ENUM_VALUE( "KeyNum", "O", KEY_O );
		REGISTER_ENUM_VALUE( "KeyNum", "P", KEY_P );
		REGISTER_ENUM_VALUE( "KeyNum", "Q", KEY_Q );
		REGISTER_ENUM_VALUE( "KeyNum", "R", KEY_R );
		REGISTER_ENUM_VALUE( "KeyNum", "S", KEY_S );
		REGISTER_ENUM_VALUE( "KeyNum", "T", KEY_T );
		REGISTER_ENUM_VALUE( "KeyNum", "U", KEY_U );
		REGISTER_ENUM_VALUE( "KeyNum", "V", KEY_V );
		REGISTER_ENUM_VALUE( "KeyNum", "W", KEY_W );
		REGISTER_ENUM_VALUE( "KeyNum", "X", KEY_X );
		REGISTER_ENUM_VALUE( "KeyNum", "Y", KEY_Y );
		REGISTER_ENUM_VALUE( "KeyNum", "Z", KEY_Z );
		REGISTER_ENUM_VALUE( "KeyNum", "Tab", KEY_TAB );
		REGISTER_ENUM_VALUE( "KeyNum", "Space", KEY_SPACE );
		REGISTER_ENUM_VALUE( "KeyNum", "BackSpace", KEY_BACKSPACE );
		REGISTER_ENUM_VALUE( "KeyNum", "Alt", KEY_ALT );
		REGISTER_ENUM_VALUE( "KeyNum", "UpArrow", KEY_UPARROW );
		REGISTER_ENUM_VALUE( "KeyNum", "LeftArrow", KEY_LEFTARROW );
		REGISTER_ENUM_VALUE( "KeyNum", "DownArrow", KEY_DOWNARROW );
		REGISTER_ENUM_VALUE( "KeyNum", "RightArrow", KEY_RIGHTARROW );
		REGISTER_ENUM_VALUE( "KeyNum", "BackSlash", KEY_BACKSLASH );
		REGISTER_ENUM_VALUE( "KeyNum", "Slash", KEY_SLASH );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_A", KEY_PAD0_A );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_B", KEY_PAD0_B );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_X", KEY_PAD0_X );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_Y", KEY_PAD0_Y );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_Start", KEY_PAD0_START );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_Guide", KEY_PAD0_GUIDE );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_LeftButton", KEY_PAD0_LEFTBUTTON );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_RightButton", KEY_PAD0_RIGHTBUTTON );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_LeftTrigger", KEY_PAD0_LEFTTRIGGER );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_RightTrigger", KEY_PAD0_RIGHTTRIGGER );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_LeftStick_Click", KEY_PAD0_LEFTSTICK_CLICK );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_LeftStick_Up", KEY_PAD0_LEFTSTICK_UP );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_LeftStick_Down", KEY_PAD0_LEFTSTICK_DOWN );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_LeftStick_Left", KEY_PAD0_LEFTSTICK_LEFT );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_LeftStick_Right", KEY_PAD0_LEFTSTICK_RIGHT );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_RightStick_Click", KEY_PAD0_RIGHTSTICK_CLICK );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_RightStick_Up", KEY_PAD0_RIGHTSTICK_UP );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_RightStick_Down", KEY_PAD0_RIGHTSTICK_DOWN );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_RightStick_Left", KEY_PAD0_RIGHTSTICK_LEFT );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_RightStick_Right", KEY_PAD0_RIGHTSTICK_RIGHT );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_DPad_Up", KEY_PAD0_DPAD_UP );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_DPad_Down", KEY_PAD0_DPAD_DOWN );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_DPad_Left", KEY_PAD0_DPAD_LEFT );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_DPad_Right", KEY_PAD0_DPAD_RIGHT );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_Back", KEY_PAD0_BACK );
		
		// only the Xbox Elite Controller uses these, just added for those kinds of people...
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_Paddle1", KEY_PAD0_PADDLE1 );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_Paddle2", KEY_PAD0_PADDLE2 );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_Paddle3", KEY_PAD0_PADDLE3 );
		REGISTER_ENUM_VALUE( "KeyNum", "GamePad_Paddle4", KEY_PAD0_PADDLE4 );
	}

	{
		REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::CvarVariableInteger( const string& in )", asFUNCTION( CvarVariableInteger ),
			asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "float TheNomad::Engine::CvarVariableFloat( const string& in )", asFUNCTION( CvarVariableFloat ),
			asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "const string TheNomad::Engine::CvarVariableString( const string& in )", asFUNCTION( CvarVariableString ),
			asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CvarRegister( const string& in, string& in, uint, int32& out, float& out, int& out, int& out )",
			asFUNCTION( CvarRegister ), asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CvarUpdate( string& out value, int& out, float& out, int32& out, int )",
			asFUNCTION( CvarUpdate ), asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CvarSet( const string& in, const string& in )", asFUNCTION( CvarSet ), asCALL_CDECL );
	}
	{
		REGISTER_GLOBAL_FUNCTION( "const string TheNomad::Engine::CmdArgv( uint )", asFUNCTION( CmdArgv ), asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::CmdArgc()", asFUNCTION( Cmd_Argc ), asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CmdAddCommand( const string& in )", asFUNCTION( CmdAddCommand ), asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CmdRemoveCommand( const string& in )", asFUNCTION( CmdRemoveCommand ), asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CmdExecuteCommand( const string& in )", asFUNCTION( CmdExecuteCommand ), asCALL_CDECL );
	}
	{
		REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::IsAnyKeyDown()", asFUNCTION( IsAnyKeyDown ), asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::IsKeyDown( KeyNum )", asFUNCTION( IsKeyDown ), asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::KeyGetKey( const string& in )", asFUNCTION( KeyGetKey ), asCALL_CDECL );
		REGISTER_GLOBAL_FUNCTION( "const string TheNomad::Engine::KeyGetBinding( uint )", asFUNCTION( KeyGetBinding ), asCALL_CDECL );

	}
	RESET_NAMESPACE();

	/*
	SET_NAMESPACE( "TheNomad::Util" );
	{
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Util::StrICmp()",  );
	}
	RESET_NAMESPACE();
	*/
}