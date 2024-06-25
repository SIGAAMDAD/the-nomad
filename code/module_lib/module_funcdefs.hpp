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

#ifndef __MODULE_FUNCDEFS__
#define __MODULE_FUNCDEFS__

#pragma once

//
// ModuleException: we want to throw an exception from the vm to get the modulelib to
// shut it down
//
class ModuleException : public std::exception {
public:
    ModuleException() = default;
    ModuleException( const char *msg )
        : m_szMessage{ msg }
    {
        Cvar_Set( "com_errorMessage", msg );
    }
    ModuleException( const string_t *msg )
        : m_szMessage{ eastl::move( *msg ) }
    {
        Cvar_Set( "com_errorMessage", msg->c_str() );
    }
    ModuleException( const ModuleException& ) = default;
    ModuleException( ModuleException&& ) = default;
    virtual ~ModuleException() = default;

    ModuleException& operator=( const ModuleException& ) = default;
    ModuleException& operator=( ModuleException&& ) = default;

    const char *what( void ) const noexcept {
        return m_szMessage.c_str();
    }
private:
    string_t m_szMessage;
};

// glm has a lot of very fuzzy template types
using vec2 = glm::vec<2, float, glm::packed_highp>;
using vec3 = glm::vec<3, float, glm::packed_highp>;
using vec4 = glm::vec<4, float, glm::packed_highp>;
using ivec2 = glm::vec<2, int, glm::packed_highp>;
using ivec3 = glm::vec<3, int, glm::packed_highp>;
using ivec4 = glm::vec<4, int, glm::packed_highp>;
using uvec2 = glm::vec<2, unsigned, glm::packed_highp>;
using uvec3 = glm::vec<3, unsigned, glm::packed_highp>;
using uvec4 = glm::vec<4, unsigned, glm::packed_highp>;

void ModuleLib_Register_Util( void );
void ModuleLib_Register_Cvar( void );
void ModuleLib_Register_RenderEngine( void );
void ModuleLib_Register_Engine( void );
void ModuleLib_Register_FileSystem( void );
void ModuleLib_Register_SoundSystem( void );

#endif