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

#ifndef __MODULE_PRAGMAHANDLERS__
#define __MODULE_PRAGMAHANDLERS__

#include "module_public.h"
#include "scriptpreprocessor.h"

class CIgnoreWarningPragma : public Preprocessor::PragmaModel
{
public:
    CIgnoreWarningPragma( void ) = default;
    virtual ~CIgnoreWarningPragma() override = default;

    virtual void handlePragma( const Preprocessor::PragmaInstance& pragma ) {
        Con_Printf( "Handling pragma %s\n", pragma.text.c_str() );
    }
};

#endif