#ifndef __MODULE_BUFFER__
#define __MODULE_BUFFER__

#pragma once

#include "../engine/n_shared.h"

class CModuleBuffer
{
public:
    CModuleBuffer( uint32_t nBytes );
    ~CModuleBuffer();

    uint32_t GetSize( void ) const;
    void Reserve( uint32_t nItems );
    void SetSize( uint32_t nBytes );
    void SetMemory( const void *pBuffer, uint32_t nBytes );
    void GetMemory( void *pBuffer, uint32_t nBytes ) const;
private:
    void *m_pBuffer;
    uint32_t m_nSize;
};

#endif