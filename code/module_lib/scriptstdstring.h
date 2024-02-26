//
// Script std::string
//
// This function registers the std::string type with AngelScript to be used as the default string type.
//
// The string type is registered as a value type, thus may have performance issues if a lot of 
// string operations are performed in the script. However, for relatively few operations, this should
// not cause any problem for most applications.
//

#ifndef SCRIPTSTDSTRING_H
#define SCRIPTSTDSTRING_H

#include "module_public.h"
#include <EASTL/string.h>

using string_t = eastl::basic_string<char, eastl::allocator_malloc<char>>;

//---------------------------
// Compilation settings
//

// Sometimes it may be desired to use the same method names as used by C++ STL.
// This may for example reduce time when converting code from script to C++ or
// back.
//
//  0 = off
//  1 = on
#ifndef AS_USE_STLNAMES
#define AS_USE_STLNAMES 0
#endif

// Some prefer to use property accessors to get/set the length of the string
// This option registers the accessors instead of the method length()
#ifndef AS_USE_ACCESSORS
#define AS_USE_ACCESSORS 0
#endif

void RegisterStdString_Generic(asIScriptEngine *engine);
void RegisterStdString(asIScriptEngine *engine);
void RegisterStdStringUtils(asIScriptEngine *engine);

#endif
