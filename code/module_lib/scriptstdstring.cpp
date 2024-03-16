#include "scriptstdstring.h"
#ifndef __psp2__
	#include <locale.h> // setlocale()
#endif
#include "../engine/n_threads.h"

#include "module_stringfactory.hpp"

CModuleStringFactory *g_pStringFactory;

CModuleStringFactory *GetStringFactorySingleton( void )
{
	if ( !g_pStringFactory ) {
		CThreadMutex mutex;
		
		CThreadAutoLock<CThreadMutex> lock( mutex );
		if ( !g_pStringFactory ) {
			g_pStringFactory = CreateObject<CModuleStringFactory>();
		}
	}
	return g_pStringFactory;
}

// This macro is used to avoid warnings about unused variables.
// Usually where the variables are only used in debug mode.
#define UNUSED_VAR(x) (void)(x)

static void ConstructString(string_t *thisPointer)
{
	new(thisPointer) string_t();
}

static void CopyConstructString(const string_t& other, string_t *thisPointer)
{
	new(thisPointer) string_t(other);
}

static void DestructString(string_t *thisPointer)
{
	thisPointer->~string_t();
}

static string_t& AddAssignStringToString(const string_t& str, string_t& dest)
{
	// We don't register the method directly because some compilers
	// and standard libraries inline the definition, resulting in the
	// linker being unable to find the declaration.
	// Example: CLang/LLVM with XCode 4.3 on OSX 10.7
	dest += str;
	return dest;
}

// bool string::isEmpty()
// bool string::empty() // if AS_USE_STLNAMES == 1
static bool StringIsEmpty(const string_t& str)
{
	// We don't register the method directly because some compilers
	// and standard libraries inline the definition, resulting in the
	// linker being unable to find the declaration
	// Example: CLang/LLVM with XCode 4.3 on OSX 10.7
	return str.empty();
}

static string_t& AssignUInt64ToString(asQWORD i, string_t& dest)
{
	dest.sprintf( "%lu", i );
	return dest;
}

static string_t& AddAssignUInt64ToString(asQWORD i, string_t& dest)
{
	dest.append_sprintf( "%lu", i );
	return dest;
}

static string_t AddStringUInt64(const string_t& str, asQWORD i)
{
	return str + va( "%lu", i );
}

static string_t AddInt64String(asINT64 i, const string_t& str)
{
	return string_t( va( "%li", i ) ) + str;
}

static string_t& AssignInt64ToString(asINT64 i, string_t& dest)
{
	dest.sprintf( "%li", i );
	return dest;
}

static string_t& AddAssignInt64ToString(asINT64 i, string_t& dest)
{
	dest.append_sprintf( "%li", i );
	return dest;
}

static string_t AddStringInt64(const string_t& str, asINT64 i)
{
	return str + va( "%li", i );
}

static string_t AddUInt64String(asQWORD i, const string_t& str)
{
	return string_t( va( "%li", i ) ) + str;
}

static string_t& AssignDoubleToString(double f, string_t& dest)
{
	dest.sprintf( "%lf", f );
	return dest;
}

static string_t& AddAssignDoubleToString(double f, string_t& dest)
{
	dest.append_sprintf( "%lf", f );
	return dest;
}

static string_t& AssignFloatToString(float f, string_t& dest)
{
	dest.sprintf( "%f", f );
	return dest;
}

static string_t& AddAssignFloatToString(float f, string_t& dest)
{
	dest.append_sprintf( "%f", f );
	return dest;
}

static string_t& AssignBoolToString(bool b, string_t& dest)
{
	dest.sprintf( "%s", b ? "true" : "false" );
	return dest;
}

static string_t& AddAssignBoolToString(bool b, string_t& dest)
{
	dest.append_sprintf( "%s", b ? "true" : "false" );
	return dest;
}

static string_t AddStringDouble(const string_t& str, double f)
{
	return str + va( "%lf", f );
}

static string_t AddDoubleString(double f, const string_t& str)
{
	return string_t( va( "%lf", f ) ) + str;
}

static string_t AddStringFloat(const string_t& str, float f)
{
	return str + va( "%f", f );
}

static string_t AddFloatString(float f, const string_t& str)
{
	return string_t( va( "%f", f ) )  + str;
}

static string_t AddStringBool(const string_t& str, bool b)
{
	return str + va( "%s", b ? "true" : "false" );
}

static string_t AddBoolString(bool b, const string_t& str)
{
	return string_t( va( "%s", b ? "true" : "false" ) ) + str;
}

static char *StringCharAt( asQWORD i, string_t& str)
{
	if( i >= str.size() )
	{
		// Set a script exception
		asIScriptContext *ctx = asGetActiveContext();
		ctx->SetException( "Out of range" );

		return NULL;
	}

	return &str[i];
}

// AngelScript signature:
// int string::opCmp(const string_t& in) const
static asQWORD StringCmp(const string_t& a, const string_t& b)
{
	return N_strcmp( a.c_str(), b.c_str() );
}

// This function returns the index of the first position where the substring
// exists in the input string. If the substring doesn't exist in the input
// string_t -1 is returned.
//
// AngelScript signature:
// int string::findFirst(const string_t& in sub, uint start = 0) const
static asQWORD StringFindFirst(const string_t& sub, asQWORD start, const string_t& str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (asQWORD)str.find(sub, (asQWORD)(start < 0 ? string_t::npos : start));
}

// This function returns the index of the first position where the one of the bytes in substring
// exists in the input string. If the characters in the substring doesn't exist in the input
// string_t -1 is returned.
//
// AngelScript signature:
// int string::findFirstOf(const string_t& in sub, uint start = 0) const
static asQWORD StringFindFirstOf(const string_t& sub, asQWORD start, const string_t& str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (asQWORD)str.find_first_of(sub, (asQWORD)(start < 0 ? string_t::npos : start));
}

// This function returns the index of the last position where the one of the bytes in substring
// exists in the input string. If the characters in the substring doesn't exist in the input
// string_t -1 is returned.
//
// AngelScript signature:
// int string::findLastOf(const string_t& in sub, uint start = -1) const
static asQWORD StringFindLastOf(const string_t& sub, asQWORD start, const string_t& str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (asQWORD)str.find_last_of(sub, (asQWORD)(start < 0 ? string_t::npos : start));
}

// This function returns the index of the first position where a byte other than those in substring
// exists in the input string. If none is found -1 is returned.
//
// AngelScript signature:
// int string::findFirstNotOf(const string_t& in sub, uint start = 0) const
static asQWORD StringFindFirstNotOf(const string_t& sub, asQWORD start, const string_t& str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (asQWORD)str.find_first_not_of(sub, (asQWORD)(start < 0 ? string_t::npos : start));
}

// This function returns the index of the last position where a byte other than those in substring
// exists in the input string. If none is found -1 is returned.
//
// AngelScript signature:
// int string::findLastNotOf(const string_t& in sub, uint start = -1) const
static asQWORD StringFindLastNotOf(const string_t& sub, asQWORD start, const string_t& str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (asQWORD)str.find_last_not_of(sub, (asQWORD)(start < 0 ? string_t::npos : start));
}

// This function returns the index of the last position where the substring
// exists in the input string. If the substring doesn't exist in the input
// string_t -1 is returned.
//
// AngelScript signature:
// int string::findLast(const string_t& in sub, int start = -1) const
static asQWORD StringFindLast(const string_t& sub, asQWORD start, const string_t& str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (asQWORD)str.rfind(sub, (asQWORD)(start < 0 ? string_t::npos : start));
}

// AngelScript signature:
// void string::insert(uint pos, const string_t& in other)
static void StringInsert( asQWORD pos, const string_t& other, string_t& str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	str.insert(pos, other);
}

// AngelScript signature:
// void string::erase(uint pos, int count = -1)
static void StringErase( asQWORD pos, asQWORD count, string_t& str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	str.erase(pos, (asQWORD)(count < 0 ? string_t::npos : count));
}


// AngelScript signature:
// uint string::length() const
static asQWORD StringLength(const string_t& str)
{
	// We don't register the method directly because the return type changes between 32bit and 64bit platforms
	return str.length();
}


// AngelScript signature:
// void string::resize(uint l)
static void StringResize(asQWORD l, string_t& str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	str.resize(l);
}

// AngelScript signature:
// string_t formatInt(int64 val, const string_t& in options, uint width)
static string_t formatInt(asINT64 value, const string_t& options, asQWORD width)
{
	const bool leftJustify = options.find("l") != string_t::npos;
	const bool padWithZero = options.find("0") != string_t::npos;
	const bool alwaysSign  = options.find("+") != string_t::npos;
	const bool spaceOnSign = options.find(" ") != string_t::npos;
	const bool hexSmall    = options.find("h") != string_t::npos;
	const bool hexLarge    = options.find("H") != string_t::npos;
	string_t fmt = "%";

	if ( leftJustify ) fmt.append( "-" );
	if ( alwaysSign ) fmt.append( "+" );
	if ( spaceOnSign ) fmt.append( " " );
	if ( padWithZero ) fmt.append( "0" );

#ifdef _WIN32
	fmt.append( "*I64" );
#else
#ifdef _LP64
	fmt.append( "*l" );
#else
	fmt += "*ll";
#endif
#endif

	if( hexSmall ) fmt += "x";
	else if( hexLarge ) fmt += "X";
	else fmt += "d";

	string_t buf;
	buf.resize( width + 30 );
#if _MSC_VER >= 1400 && !defined(__S3E__)
	// MSVC 8.0 / 2005 or newer
	sprintf_s(&buf[0], buf.size(), fmt.c_str(), width, value);
#else
	sprintf(&buf[0], fmt.c_str(), width, value);
#endif
	buf.resize(strlen(&buf[0]));

	return buf;
}

// AngelScript signature:
// string_t formatUInt(uint64 val, const string_t& in options, uint width)
static string_t formatUInt(asQWORD value, const string_t& options, asQWORD width)
{
	bool leftJustify = options.find("l") != string_t::npos;
	bool padWithZero = options.find("0") != string_t::npos;
	bool alwaysSign  = options.find("+") != string_t::npos;
	bool spaceOnSign = options.find(" ") != string_t::npos;
	bool hexSmall    = options.find("h") != string_t::npos;
	bool hexLarge    = options.find("H") != string_t::npos;

	string_t fmt = "%";
	if( leftJustify ) fmt += "-";
	if( alwaysSign ) fmt += "+";
	if( spaceOnSign ) fmt += " ";
	if( padWithZero ) fmt += "0";

#ifdef _WIN32
	fmt += "*I64";
#else
#ifdef _LP64
	fmt += "*l";
#else
	fmt += "*ll";
#endif
#endif

	if( hexSmall ) fmt += "x";
	else if( hexLarge ) fmt += "X";
	else fmt += "u";

	string_t buf;
	buf.resize(width+30);
#if _MSC_VER >= 1400 && !defined(__S3E__)
	// MSVC 8.0 / 2005 or newer
	sprintf_s(&buf[0], buf.size(), fmt.c_str(), width, value);
#else
	sprintf(&buf[0], fmt.c_str(), width, value);
#endif
	buf.resize(strlen(&buf[0]));

	return buf;
}

// AngelScript signature:
// string_t formatFloat(double val, const string_t& in options, uint width, uint precision)
static string_t formatFloat(double value, const string_t& options, asUINT width, asUINT precision)
{
	bool leftJustify = options.find("l") != string_t::npos;
	bool padWithZero = options.find("0") != string_t::npos;
	bool alwaysSign  = options.find("+") != string_t::npos;
	bool spaceOnSign = options.find(" ") != string_t::npos;
	bool expSmall    = options.find("e") != string_t::npos;
	bool expLarge    = options.find("E") != string_t::npos;

	string_t fmt = "%";
	if( leftJustify ) fmt += "-";
	if( alwaysSign ) fmt += "+";
	if( spaceOnSign ) fmt += " ";
	if( padWithZero ) fmt += "0";

	fmt += "*.*";

	if( expSmall ) fmt += "e";
	else if( expLarge ) fmt += "E";
	else fmt += "f";

	string_t buf;
	buf.resize(width+precision+50);
#if _MSC_VER >= 1400 && !defined(__S3E__)
	// MSVC 8.0 / 2005 or newer
	sprintf_s(&buf[0], buf.size(), fmt.c_str(), width, precision, value);
#else
	sprintf(&buf[0], fmt.c_str(), width, precision, value);
#endif
	buf.resize(strlen(&buf[0]));

	return buf;
}

// AngelScript signature:
// int64 parseInt(const string_t& in val, uint base = 10, uint &out byteCount = 0)
static asINT64 parseInt(const string_t& val, asUINT base, asUINT *byteCount)
{
	// Only accept base 10 and 16
	if( base != 10 && base != 16 )
	{
		if( byteCount ) *byteCount = 0;
		return 0;
	}

	const char *end = &val[0];

	// Determine the sign
	bool sign = false;
	if( *end == '-' )
	{
		sign = true;
		end++;
	}
	else if( *end == '+' )
		end++;

	asINT64 res = 0;
	if( base == 10 )
	{
		while( *end >= '0' && *end <= '9' )
		{
			res *= 10;
			res += *end++ - '0';
		}
	}
	else if( base == 16 )
	{
		while( (*end >= '0' && *end <= '9') ||
		       (*end >= 'a' && *end <= 'f') ||
		       (*end >= 'A' && *end <= 'F') )
		{
			res *= 16;
			if( *end >= '0' && *end <= '9' )
				res += *end++ - '0';
			else if( *end >= 'a' && *end <= 'f' )
				res += *end++ - 'a' + 10;
			else if( *end >= 'A' && *end <= 'F' )
				res += *end++ - 'A' + 10;
		}
	}

	if( byteCount )
		*byteCount = asUINT(asQWORD(end - val.c_str()));

	if( sign )
		res = -res;

	return res;
}

// AngelScript signature:
// uint64 parseUInt(const string_t& in val, uint base = 10, uint &out byteCount = 0)
static asQWORD parseUInt(const string_t& val, asUINT base, asUINT *byteCount)
{
	// Only accept base 10 and 16
	if (base != 10 && base != 16)
	{
		if (byteCount) *byteCount = 0;
		return 0;
	}

	const char *end = &val[0];

	asQWORD res = 0;
	if (base == 10)
	{
		while (*end >= '0' && *end <= '9')
		{
			res *= 10;
			res += *end++ - '0';
		}
	}
	else if (base == 16)
	{
		while ((*end >= '0' && *end <= '9') ||
			(*end >= 'a' && *end <= 'f') ||
			(*end >= 'A' && *end <= 'F'))
		{
			res *= 16;
			if (*end >= '0' && *end <= '9')
				res += *end++ - '0';
			else if (*end >= 'a' && *end <= 'f')
				res += *end++ - 'a' + 10;
			else if (*end >= 'A' && *end <= 'F')
				res += *end++ - 'A' + 10;
		}
	}

	if (byteCount)
		*byteCount = asUINT(asQWORD(end - val.c_str()));

	return res;
}

// AngelScript signature:
// double parseFloat(const string_t& in val, uint &out byteCount = 0)
double parseFloat(const string_t& val, asUINT *byteCount)
{
	char *end;

	// WinCE doesn't have setlocale. Some quick testing on my current platform
	// still manages to parse the numbers such as "3.14" even if the decimal for the
	// locale is ",".
#if !defined(_WIN32_WCE) && !defined(ANDROID) && !defined(__psp2__)
	// Set the locale to C so that we are guaranteed to parse the float value correctly
	char *tmp = setlocale(LC_NUMERIC, 0);
	string_t orig = tmp ? tmp : "C";
	setlocale(LC_NUMERIC, "C");
#endif

	double res = strtod(val.c_str(), &end);

#if !defined(_WIN32_WCE) && !defined(ANDROID) && !defined(__psp2__)
	// Restore the locale
	setlocale(LC_NUMERIC, orig.c_str());
#endif

	if( byteCount )
		*byteCount = asUINT(asQWORD(end - val.c_str()));

	return res;
}

// This function returns a string_t containing the substring of the input string
// determined by the starting index and count of characters.
//
// AngelScript signature:
// string_t string::substr(uint start = 0, int count = -1) const
static string_t StringSubString(asUINT start, int count, const string_t& str)
{
	// Check for out-of-bounds
	string_t ret;
	if( start < str.length() && count != 0 )
		ret = str.substr(start, (asQWORD)(count < 0 ? string_t::npos : count));

	return ret;
}

// String equality comparison.
// Returns true iff lhs is equal to rhs.
//
// For some reason gcc 4.7 has difficulties resolving the
// asFUNCTIONPR(operator==, (const string_t& , const string_t& )
// makro, so this wrapper was introduced as work around.
static bool StringEquals(const string_t& lhs, const string_t& rhs)
{
	return N_strcmp( lhs.c_str(), rhs.c_str() ) == 0;
}

static string_t StringAppend( const string_t& value, const string_t& add )
{
	return value + add;
}

void RegisterStdString_Native(asIScriptEngine *engine)
{
	int r = 0;
	UNUSED_VAR(r);

	// Register the string_t type
#if AS_CAN_USE_CPP11
	// With C++11 it is possible to use asGetTypeTraits to automatically determine the correct flags to use
	r = engine->RegisterObjectType("string", sizeof(string_t), asOBJ_VALUE | asGetTypeTraits<string_t>()); assert( r >= 0 );
#else
	r = engine->RegisterObjectType("string", sizeof(string_t), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert( r >= 0 );
#endif

	r = engine->RegisterStringFactory("string", GetStringFactorySingleton());

	// Register the object operator overloads
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(ConstructString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f(const string& in)",    asFUNCTION(CopyConstructString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT,   "void f()",                    asFUNCTION(DestructString),  asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAssign(const string& in)", asMETHODPR(string_t, operator =, (const string_t&), string_t&), asCALL_THISCALL); assert( r >= 0 );
	// Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM, otherwise the linker fails
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(const string& in)", asFUNCTION(AddAssignStringToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
//	r = engine->RegisterObjectMethod("string", "string &opAddAssign(const string& in)", asMETHODPR(string, operator+=, (const string&), string&), asCALL_THISCALL); assert( r >= 0 );

	// Need to use a wrapper for operator== otherwise gcc 4.7 fails to compile
	r = engine->RegisterObjectMethod("string", "bool opEquals(const string& in) const", asFUNCTIONPR(StringEquals, (const string_t& , const string_t& ), bool), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int opCmp(const string& in) const", asFUNCTION(StringCmp), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(const string& in) const", asFUNCTION(StringAppend), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

	// The string_t length can be accessed through methods or through virtual property
	// TODO: Register as size() for consistency with other types
#if AS_USE_ACCESSORS != 1
	r = engine->RegisterObjectMethod("string", "uint length() const", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST); assert( r >= 0 );
#endif
	r = engine->RegisterObjectMethod("string", "void resize(uint)", asFUNCTION(StringResize), asCALL_CDECL_OBJLAST); assert( r >= 0 );
#if AS_USE_STLNAMES != 1 && AS_USE_ACCESSORS == 1
	// Don't register these if STL names is used, as they conflict with the method size()
	r = engine->RegisterObjectMethod("string", "uint length() const property", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "void resize(uint) property", asFUNCTION(StringResize), asCALL_CDECL_OBJLAST); assert( r >= 0 );
#endif
	// Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM, otherwise the linker fails
//	r = engine->RegisterObjectMethod("string", "bool isEmpty() const", asMETHOD(string, empty), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool isEmpty() const", asFUNCTION(StringIsEmpty), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Register the index operator, both as a mutator and as an inspector
	// Note that we don't register the operator[] directly, as it doesn't do bounds checking
	r = engine->RegisterObjectMethod("string", "uint8 &opIndex(uint)", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "const uint8 &opIndex(uint) const", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Automatic conversion from values
	r = engine->RegisterObjectMethod("string", "string &opAssign(double)", asFUNCTION(AssignDoubleToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(double)", asFUNCTION(AddAssignDoubleToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(double) const", asFUNCTION(AddStringDouble), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(double) const", asFUNCTION(AddDoubleString), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(float)", asFUNCTION(AssignFloatToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(float)", asFUNCTION(AddAssignFloatToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(float) const", asFUNCTION(AddStringFloat), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(float) const", asFUNCTION(AddFloatString), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(int64)", asFUNCTION(AssignInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(int64)", asFUNCTION(AddAssignInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(int64) const", asFUNCTION(AddStringInt64), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(int64) const", asFUNCTION(AddInt64String), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(uint64)", asFUNCTION(AssignUInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(uint64)", asFUNCTION(AddAssignUInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(uint64) const", asFUNCTION(AddStringUInt64), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(uint64) const", asFUNCTION(AddUInt64String), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(bool)", asFUNCTION(AssignBoolToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(bool)", asFUNCTION(AddAssignBoolToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(bool) const", asFUNCTION(AddStringBool), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(bool) const", asFUNCTION(AddBoolString), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Utilities
	r = engine->RegisterObjectMethod("string", "string substr(uint start = 0, int count = -1) const", asFUNCTION(StringSubString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int findFirst(const string& in, uint start = 0) const", asFUNCTION(StringFindFirst), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int findFirstOf(const string&, uint start = 0) const", asFUNCTION(StringFindFirstOf), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findFirstNotOf(const string&, uint start = 0) const", asFUNCTION(StringFindFirstNotOf), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findLast(const string&, int start = -1) const", asFUNCTION(StringFindLast), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int findLastOf(const string&, int start = -1) const", asFUNCTION(StringFindLastOf), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findLastNotOf(const string&, int start = -1) const", asFUNCTION(StringFindLastNotOf), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "void insert(uint pos, const string& in other)", asFUNCTION(StringInsert), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "void erase(uint pos, int count = -1)", asFUNCTION(StringErase), asCALL_CDECL_OBJLAST); assert(r >= 0);


	r = engine->RegisterGlobalFunction("string formatInt(int64 val, const string& in options = \"\", uint width = 0)", asFUNCTION(formatInt), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("string formatUInt(uint64 val, const string& in options = \"\", uint width = 0)", asFUNCTION(formatUInt), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("string formatFloat(double val, const const string& in options = \"\", uint width = 0, uint precision = 0)", asFUNCTION(formatFloat), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int64 parseInt(const string& in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseInt), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint64 parseUInt(const string& in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseUInt), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("double parseFloat(const string& in, uint &out byteCount = 0)", asFUNCTION(parseFloat), asCALL_CDECL); assert(r >= 0);

	// Same as length
	r = engine->RegisterObjectMethod("string", "uint size() const", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	// Same as isEmpty
	r = engine->RegisterObjectMethod("string", "bool empty() const", asFUNCTION(StringIsEmpty), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	// Same as findFirst
	r = engine->RegisterObjectMethod("string", "int find(const string& in, uint start = 0) const", asFUNCTION(StringFindFirst), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	// Same as findLast
	r = engine->RegisterObjectMethod("string", "int rfind(const string& in, int start = -1) const", asFUNCTION(StringFindLast), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// TODO: Implement the following
	// findAndReplace - replaces a text found in the string
	// replaceRange - replaces a range of bytes in the string
	// multiply/times/opMul/opMul_r - takes the string_t and multiplies it n times, e.g. "-".multiply(5) returns "-----"
}

static void ConstructStringGeneric( asIScriptGeneric *gen ) {
	new ( gen->GetObjectData() ) string_t();
}

static void CopyConstructStringGeneric(asIScriptGeneric *gen)
{
	string_t *a = static_cast<string_t *>(gen->GetArgObject(0));
	new (gen->GetObjectData()) string_t(*a);
}

static void DestructStringGeneric(asIScriptGeneric *gen)
{
	string_t *ptr = static_cast<string_t *>(gen->GetObjectData());
	ptr->~string_t();
}

static void AssignStringGeneric(asIScriptGeneric *gen)
{
	string_t *a = static_cast<string_t *>(gen->GetArgObject(0));
	string_t *self = static_cast<string_t *>(gen->GetObjectData());
	*self = *a;
	gen->SetReturnAddress(self);
}

static void AddAssignStringGeneric(asIScriptGeneric *gen)
{
	string_t *a = static_cast<string_t *>(gen->GetArgObject(0));
	string_t *self = static_cast<string_t *>(gen->GetObjectData());
	*self += *a;
	gen->SetReturnAddress(self);
}

static void StringEqualsGeneric(asIScriptGeneric *gen)
{
	string_t *a = static_cast<string_t *>(gen->GetObjectData());
	string_t *b = static_cast<string_t *>(gen->GetArgAddress(0));
	*(bool*)gen->GetAddressOfReturnLocation() = (*a == *b);
}

static void StringCmpGeneric(asIScriptGeneric *gen)
{
	string_t *a = static_cast<string_t *>(gen->GetObjectData());
	string_t *b = static_cast<string_t *>(gen->GetArgAddress(0));

	int cmp = 0;
	if( *a < *b ) cmp = -1;
	else if( *a > *b ) cmp = 1;

	*(int*)gen->GetAddressOfReturnLocation() = cmp;
}

static void StringAddGeneric(asIScriptGeneric *gen)
{
	string_t *a = static_cast<string_t *>(gen->GetObjectData());
	string_t *b = static_cast<string_t *>(gen->GetArgAddress(0));
	string_t ret_val = *a + *b;
	gen->SetReturnObject(&ret_val);
}

static void StringLengthGeneric(asIScriptGeneric *gen)
{
	string_t *self = static_cast<string_t *>(gen->GetObjectData());
	*static_cast<asUINT *>(gen->GetAddressOfReturnLocation()) = (asUINT)self->length();
}

static void StringIsEmptyGeneric(asIScriptGeneric *gen)
{
	string_t *self = reinterpret_cast<string_t *>(gen->GetObjectData());
	*reinterpret_cast<bool *>(gen->GetAddressOfReturnLocation()) = StringIsEmpty(*self);
}

static void StringResizeGeneric(asIScriptGeneric *gen)
{
	string_t *self = static_cast<string_t *>(gen->GetObjectData());
	self->resize(*static_cast<asUINT *>(gen->GetAddressOfArg(0)));
}

static void StringReserveGeneric( asIScriptGeneric *gen )
{
	string_t *self = (string_t *)gen->GetObjectData();
	self->reserve( *(asUINT *)gen->GetAddressOfArg( 0 ) );
}

static void StringInsert_Generic(asIScriptGeneric *gen)
{
	string_t *self = static_cast<string_t *>(gen->GetObjectData());
	asUINT pos = gen->GetArgDWord(0);
	string_t *other = reinterpret_cast<string_t *>(gen->GetArgAddress(1));
	StringInsert(pos, *other, *self);
}

static void StringErase_Generic(asIScriptGeneric *gen)
{
	string_t *self = static_cast<string_t *>(gen->GetObjectData());
	asUINT pos = gen->GetArgDWord(0);
	int count = int(gen->GetArgDWord(1));
	StringErase(pos, count, *self);
}

static void StringFindFirst_Generic(asIScriptGeneric *gen)
{
	string_t *find = reinterpret_cast<string_t*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	string_t *self = reinterpret_cast<string_t *>(gen->GetObjectData());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindFirst(*find, start, *self);
}

static void StringFindLast_Generic(asIScriptGeneric *gen)
{
	string_t *find = reinterpret_cast<string_t*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	string_t *self = reinterpret_cast<string_t *>(gen->GetObjectData());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindLast(*find, start, *self);
}

static void StringFindFirstOf_Generic(asIScriptGeneric *gen)
{
	string_t *find = reinterpret_cast<string_t*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	string_t *self = reinterpret_cast<string_t *>(gen->GetObjectData());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindFirstOf(*find, start, *self);
}

static void StringFindLastOf_Generic(asIScriptGeneric *gen)
{
	string_t *find = reinterpret_cast<string_t*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	string_t *self = reinterpret_cast<string_t *>(gen->GetObjectData());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindLastOf(*find, start, *self);
}

static void StringFindFirstNotOf_Generic(asIScriptGeneric *gen)
{
	string_t *find = reinterpret_cast<string_t*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	string_t *self = reinterpret_cast<string_t *>(gen->GetObjectData());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindFirstNotOf(*find, start, *self);
}

static void StringFindLastNotOf_Generic(asIScriptGeneric *gen)
{
	string_t *find = reinterpret_cast<string_t*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	string_t *self = reinterpret_cast<string_t *>(gen->GetObjectData());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindLastNotOf(*find, start, *self);
}

static void formatInt_Generic(asIScriptGeneric *gen)
{
	asINT64 val = gen->GetArgQWord(0);
	string_t *options = reinterpret_cast<string_t*>(gen->GetArgAddress(1));
	asUINT width = gen->GetArgDWord(2);
	new(gen->GetAddressOfReturnLocation()) string_t(formatInt(val, *options, width));
}

static void formatUInt_Generic(asIScriptGeneric *gen)
{
	asQWORD val = gen->GetArgQWord(0);
	string_t *options = reinterpret_cast<string_t*>(gen->GetArgAddress(1));
	asUINT width = gen->GetArgDWord(2);
	new(gen->GetAddressOfReturnLocation()) string_t(formatUInt(val, *options, width));
}

static void formatFloat_Generic(asIScriptGeneric *gen)
{
	double val = gen->GetArgDouble(0);
	string_t *options = reinterpret_cast<string_t*>(gen->GetArgAddress(1));
	asUINT width = gen->GetArgDWord(2);
	asUINT precision = gen->GetArgDWord(3);
	new(gen->GetAddressOfReturnLocation()) string_t(formatFloat(val, *options, width, precision));
}

static void parseInt_Generic(asIScriptGeneric *gen)
{
	string_t *str = reinterpret_cast<string_t*>(gen->GetArgAddress(0));
	asUINT base = gen->GetArgDWord(1);
	asUINT *byteCount = reinterpret_cast<asUINT*>(gen->GetArgAddress(2));
	gen->SetReturnQWord(parseInt(*str,base,byteCount));
}

static void parseUInt_Generic(asIScriptGeneric *gen)
{
	string_t *str = reinterpret_cast<string_t*>(gen->GetArgAddress(0));
	asUINT base = gen->GetArgDWord(1);
	asUINT *byteCount = reinterpret_cast<asUINT*>(gen->GetArgAddress(2));
	gen->SetReturnQWord(parseUInt(*str, base, byteCount));
}

static void parseFloat_Generic(asIScriptGeneric *gen)
{
	string_t *str = reinterpret_cast<string_t*>(gen->GetArgAddress(0));
	asUINT *byteCount = reinterpret_cast<asUINT*>(gen->GetArgAddress(1));
	gen->SetReturnDouble(parseFloat(*str,byteCount));
}

static void StringCharAtGeneric(asIScriptGeneric *gen)
{
	unsigned int index = gen->GetArgDWord(0);
	string_t *self = static_cast<string_t *>(gen->GetObjectData());

	if (index >= self->size())
	{
		// Set a script exception
		asIScriptContext *ctx = asGetActiveContext();
		ctx->SetException("Out of range");

		gen->SetReturnAddress(0);
	}
	else
	{
		gen->SetReturnAddress(&(self->operator [](index)));
	}
}

static void AssignInt2StringGeneric(asIScriptGeneric *gen)
{
	asINT64 *a = static_cast<asINT64*>(gen->GetAddressOfArg(0));
	string_t *self = static_cast<string_t*>(gen->GetObjectData());
	*self = va( "%lu", *a );
	gen->SetReturnAddress(self);
}

static void AssignUInt2StringGeneric(asIScriptGeneric *gen)
{
	asQWORD *a = static_cast<asQWORD*>(gen->GetAddressOfArg(0));
	string_t *self = static_cast<string_t*>(gen->GetObjectData());
	*self = va( "%lu", *a );
	gen->SetReturnAddress(self);
}

static void AssignDouble2StringGeneric(asIScriptGeneric *gen)
{
	double *a = static_cast<double*>(gen->GetAddressOfArg(0));
	string_t *self = static_cast<string_t*>(gen->GetObjectData());
	*self = va( "%lf", *a );
	gen->SetReturnAddress(self);
}

static void AssignFloat2StringGeneric(asIScriptGeneric *gen)
{
	float *a = static_cast<float*>(gen->GetAddressOfArg(0));
	string_t *self = static_cast<string_t*>(gen->GetObjectData());
	*self = va( "%f", *a );
	gen->SetReturnAddress(self);
}

static void AssignBool2StringGeneric(asIScriptGeneric *gen)
{
	bool *a = static_cast<bool*>(gen->GetAddressOfArg(0));
	string_t *self = static_cast<string_t*>(gen->GetObjectData());
	*self = va( "%s", *a ? "true" : "false" );
	gen->SetReturnAddress(self);
}

static void AddAssignDouble2StringGeneric(asIScriptGeneric *gen)
{
	double *a = static_cast<double *>(gen->GetAddressOfArg(0));
	string_t *self = static_cast<string_t *>(gen->GetObjectData());
	*self += va( "%lf", *a );
	gen->SetReturnAddress(self);
}

static void AddAssignFloat2StringGeneric(asIScriptGeneric *gen)
{
	float *a = static_cast<float *>(gen->GetAddressOfArg(0));
	string_t *self = static_cast<string_t *>(gen->GetObjectData());
	*self += va( "%f", *a );
	gen->SetReturnAddress(self);
}

static void AddAssignInt2StringGeneric(asIScriptGeneric *gen)
{
	asINT64 *a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
	string_t *self = static_cast<string_t *>(gen->GetObjectData());
	*self += va( "%li", *a );
	gen->SetReturnAddress(self);
}

static void AddAssignUInt2StringGeneric(asIScriptGeneric *gen)
{
	asQWORD *a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
	string_t *self = static_cast<string_t *>(gen->GetObjectData());
	*self += va("%lu", *a );
	gen->SetReturnAddress(self);
}

static void AddAssignBool2StringGeneric(asIScriptGeneric *gen)
{
	bool *a = static_cast<bool *>(gen->GetAddressOfArg(0));
	string_t *self = static_cast<string_t *>(gen->GetObjectData());
	*self += va( "%s", *a ? "true" : "false" );
	gen->SetReturnAddress(self);
}

static void AddString2DoubleGeneric(asIScriptGeneric *gen)
{
	string_t *a = static_cast<string_t *>(gen->GetObjectData());
	double *b = static_cast<double *>(gen->GetAddressOfArg(0));
	string_t ret_val = va( "%s%lf", a->c_str(), *b );
	gen->SetReturnObject(&ret_val);
}

static void AddString2FloatGeneric(asIScriptGeneric *gen)
{
	string_t *a = static_cast<string_t *>(gen->GetObjectData());
	float *b = static_cast<float *>(gen->GetAddressOfArg(0));
	string_t ret_val = va( "%s%f", a->c_str(), *b );
	gen->SetReturnObject(&ret_val);
}

static void AddString2IntGeneric(asIScriptGeneric *gen)
{
	string_t *a = static_cast<string_t *>(gen->GetObjectData());
	asINT64 *b = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
	string_t ret_val = va( "%s%li", a->c_str(), *b );
	gen->SetReturnObject(&ret_val);
}

static void AddString2UIntGeneric(asIScriptGeneric *gen)
{
	string_t *a = static_cast<string_t *>(gen->GetObjectData());
	asQWORD *b = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
	string_t ret_val = va( "%s%lu", a->c_str(), *b );
	gen->SetReturnObject(&ret_val);
}

static void AddString2BoolGeneric(asIScriptGeneric *gen)
{
	string_t *a = static_cast<string_t *>(gen->GetObjectData());
	bool *b = static_cast<bool *>(gen->GetAddressOfArg(0));
	string_t ret_val = va( "%s%s", a->c_str(), ( *b ? "true" : "false" ) );
	gen->SetReturnObject(&ret_val);
}

static void AddDouble2StringGeneric(asIScriptGeneric *gen)
{
	double*a = static_cast<double *>(gen->GetAddressOfArg(0));
	string_t *b = static_cast<string_t *>(gen->GetObjectData());
	string_t ret_val = va( "%lf%s", *a, b->c_str() );
	gen->SetReturnObject(&ret_val);
}

static void AddFloat2StringGeneric(asIScriptGeneric *gen)
{
	float*a = static_cast<float *>(gen->GetAddressOfArg(0));
	string_t *b = static_cast<string_t *>(gen->GetObjectData());

	string_t ret_val = va( "%f%s", *a, b->c_str() );
	gen->SetReturnObject(&ret_val);
}

static void AddInt2StringGeneric(asIScriptGeneric *gen)
{
	asINT64*a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
	string_t *b = static_cast<string_t *>(gen->GetObjectData());
	string_t ret_val = va( "%li%s", *a, b->c_str() );
	gen->SetReturnObject(&ret_val);
}

static void AddUInt2StringGeneric(asIScriptGeneric *gen)
{
	asQWORD*a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
	string_t *b = static_cast<string_t *>(gen->GetObjectData());
	string_t ret_val = va( "%lu%s", *a, b->c_str() );;
	gen->SetReturnObject(&ret_val);
}

static void AddBool2StringGeneric(asIScriptGeneric *gen)
{
	bool*a = static_cast<bool *>(gen->GetAddressOfArg(0));
	string_t *b = static_cast<string_t *>(gen->GetObjectData());
	string_t ret_val = va( "%s%s", ( *a ? "true" : "false" ), b->c_str() );
	gen->SetReturnObject(&ret_val);
}

static void StringSubString_Generic(asIScriptGeneric *gen)
{
	// Get the arguments
	string_t *str   = (string_t*)gen->GetObjectData();
	asUINT  start = *(int*)gen->GetAddressOfArg(0);
	int     count = *(int*)gen->GetAddressOfArg(1);

	// Return the substring
	new(gen->GetAddressOfReturnLocation()) string_t(StringSubString(start, count, *str));
}

static void StringIteratorBegin_Generic( asIScriptGeneric *gen )
{
	string_t *str = (string_t *)gen->GetObjectData();
	gen->SetReturnAddress( str->begin() );
}

static void StringIteratorEnd_Generic( asIScriptGeneric *gen )
{
	string_t *str = (string_t *)gen->GetObjectData();
	gen->SetReturnAddress( str->end() );
}

static void StringIteratorCBegin_Generic( asIScriptGeneric *gen )
{
	const string_t *str = (const string_t *)gen->GetObjectData();
	gen->SetReturnAddress( const_cast<char *>( str->cbegin() ) );
}

static void StringIteratorCEnd_Generic( asIScriptGeneric *gen )
{
	const string_t *str = (const string_t *)gen->GetObjectData();
	gen->SetReturnAddress( const_cast<char *>( str->cend() ) );
}

void RegisterStdString_Generic(asIScriptEngine *engine)
{
	// Register the string_t type
	CheckASCall( engine->RegisterObjectType("string", sizeof(string_t), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK ) );

	CheckASCall( engine->RegisterStringFactory("string", GetStringFactorySingleton()) );

	// Register the object operator overloads
	CheckASCall( engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(ConstructStringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f(const string &in)",    asFUNCTION(CopyConstructStringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT,   "void f()",                    asFUNCTION(DestructStringGeneric),  asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string &opAssign(const string &in)", asFUNCTION(AssignStringGeneric),    asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string &opAddAssign(const string &in)", asFUNCTION(AddAssignStringGeneric), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectMethod("string", "bool opEquals(const string &in) const", asFUNCTION(StringEqualsGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "int opCmp(const string &in) const", asFUNCTION(StringCmpGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string opAdd(const string &in) const", asFUNCTION(StringAddGeneric), asCALL_GENERIC ) );

	// Register the object methods
	CheckASCall( engine->RegisterObjectMethod("string", "uint length() const", asFUNCTION(StringLengthGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "uint size() const", asFUNCTION(StringLengthGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "void resize(uint)",   asFUNCTION(StringResizeGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "string", "void reserve( uint )", asFUNCTION(StringReserveGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "bool isEmpty() const", asFUNCTION(StringIsEmptyGeneric), asCALL_GENERIC ) );

	// Register the index operator, both as a mutator and as an inspector
	CheckASCall( engine->RegisterObjectMethod("string", "uint8 &opIndex(uint)", asFUNCTION(StringCharAtGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "const uint8 &opIndex(uint) const", asFUNCTION(StringCharAtGeneric), asCALL_GENERIC ) );

	// Automatic conversion from values
	CheckASCall( engine->RegisterObjectMethod("string", "string &opAssign(double)", asFUNCTION(AssignDouble2StringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string &opAddAssign(double)", asFUNCTION(AddAssignDouble2StringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string opAdd(double) const", asFUNCTION(AddString2DoubleGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string opAdd_r(double) const", asFUNCTION(AddDouble2StringGeneric), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectMethod("string", "string &opAssign(float)", asFUNCTION(AssignFloat2StringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string &opAddAssign(float)", asFUNCTION(AddAssignFloat2StringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string opAdd(float) const", asFUNCTION(AddString2FloatGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string opAdd_r(float) const", asFUNCTION(AddFloat2StringGeneric), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectMethod("string", "string &opAssign(int64)", asFUNCTION(AssignInt2StringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string &opAddAssign(int64)", asFUNCTION(AddAssignInt2StringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string opAdd(int64) const", asFUNCTION(AddString2IntGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string opAdd_r(int64) const", asFUNCTION(AddInt2StringGeneric), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectMethod("string", "string &opAssign(uint64)", asFUNCTION(AssignUInt2StringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string &opAddAssign(uint64)", asFUNCTION(AddAssignUInt2StringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string opAdd(uint64) const", asFUNCTION(AddString2UIntGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string opAdd_r(uint64) const", asFUNCTION(AddUInt2StringGeneric), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectMethod("string", "string &opAssign(bool)", asFUNCTION(AssignBool2StringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string &opAddAssign(bool)", asFUNCTION(AddAssignBool2StringGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string opAdd(bool) const", asFUNCTION(AddString2BoolGeneric), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "string opAdd_r(bool) const", asFUNCTION(AddBool2StringGeneric), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectMethod("string", "string substr(uint start = 0, int count = -1) const", asFUNCTION(StringSubString_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "int findFirst(const string &in, uint start = 0) const", asFUNCTION(StringFindFirst_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "int findFirstOf(const string &in, uint start = 0) const", asFUNCTION(StringFindFirstOf_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "int findFirstNotOf(const string &in, uint start = 0) const", asFUNCTION(StringFindFirstNotOf_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "int findLast(const string &in, int start = -1) const", asFUNCTION(StringFindLast_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "int findLastOf(const string &in, int start = -1) const", asFUNCTION(StringFindLastOf_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "int findLastNotOf(const string &in, int start = -1) const", asFUNCTION(StringFindLastNotOf_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "void insert(uint pos, const string &in other)", asFUNCTION(StringInsert_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod("string", "void erase(uint pos, int count = -1)", asFUNCTION(StringErase_Generic), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterGlobalFunction("string formatInt(int64 val, const string &in options = \"\", uint width = 0)", asFUNCTION(formatInt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterGlobalFunction("string formatUInt(uint64 val, const string &in options = \"\", uint width = 0)", asFUNCTION(formatUInt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterGlobalFunction("string formatFloat(double val, const string &in options = \"\", uint width = 0, uint precision = 0)", asFUNCTION(formatFloat_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterGlobalFunction("int64 parseInt(const string &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseInt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterGlobalFunction("uint64 parseUInt(const string &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseUInt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterGlobalFunction("double parseFloat(const string &in, uint &out byteCount = 0)", asFUNCTION(parseFloat_Generic), asCALL_GENERIC ) );
}

void RegisterStdString(asIScriptEngine *engine)
{
	if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
		RegisterStdString_Generic(engine);
	else
		RegisterStdString_Native(engine);
}


