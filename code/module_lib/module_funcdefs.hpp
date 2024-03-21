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
    ModuleException( const string_t *msg )
        : m_szMessage{ msg->c_str() }
    {
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
    UtlString m_szMessage;
};

void ModuleLib_Register_Cvar( void );
void ModuleLib_Register_RenderEngine( void );
void ModuleLib_Register_Engine( void );
void ModuleLib_Register_FileSystem( void );
void ModuleLib_Register_SoundSystem( void );

#endif