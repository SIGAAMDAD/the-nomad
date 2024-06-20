/*
===========================================================================
Copyright (C) 2023-2024 GDR Games

This file is part of The Nomad source code.

The Nomad source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

The Nomad source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifndef __MODULE_DLL__
#define __MODULE_DLL__

#pragma once

#include "module_public.h"

typedef struct {
    union {
        asIScriptFunction *pFunc;
        
    } Data;
} DynamicModuleSymbol_t;

//
// CModuleDynamicLibrary
// implements a basic dll extension factory for AngelScript
// you can feed in precompiled AngelScript binaries to this
//
class CModuleDynamicLibrary
{
public:
    CModuleDynamicLibrary( void );
    ~CModuleDynamicLibrary();
private:
    UtlVector<asIScriptFunction *> m_SymbolFuncs;
};

#endif