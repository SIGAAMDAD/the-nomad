// module_link.hpp -- interface for managing a data link between two vms

#ifndef __MODULE_LINK__
#define __MODULE_LINK__

#pragma once

#include "../engine/n_threads.h"

/*
* CModuleDataLink: manages a pool of memory dynamically sized by the vms
* that either links can write or read from, however the vms will not be
* given full direct access but instead they must hold their own copies
*/
GDR_EXPORT class CModuleDataLink
{
public:
    extern "C" CModuleDataLink( void );
    extern "C" CModuleDataLink( uint64_t nBytes );
    extern "C" ~CModuleDataLink();

    extern "C" uint64_t Size( void ) const;
    extern "C" void *GetBuffer( void );
    extern "C" const void *GetBuffer( void ) const;

    // memory management
    extern "C" void Create( vm_t *pModule0, vm_t *pModule1 );
    extern "C" void Fill( int value, uint32_t nBytes );
    extern "C" void Release( void );
    extern "C" void Resize( uint64_t nBytes );

    // I/O
    extern "C" void Read( void *pBuffer, uint32_t nBytes ) const;
    extern "C" void Write( const void *pBuffer, uint32_t nBytes );
private:
    void *m_pMemory;
    uint64_t m_nBytes;
    CThreadRWMutex m_hLock;

    vm_t *m_pModule0;
    vm_t *m_pModule1;
};

#endif