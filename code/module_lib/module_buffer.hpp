#ifndef __MODULE_BUFFER__
#define __MODULE_BUFFER__

#pragma once

class CModuleBuffer
{
public:
    extern "C" CModuleBuffer( uint32_t nBytes );
    extern "C" ~CModuleBuffer();

    extern "C" void SetSize( uint32_t nBytes );
    extern "C" void SetMemory( const void *pBuffer, uint32_t nBytes );
    extern "C" void GetMemory( void *pBuffer, uint32_t nBytes ) const;
private:
    void *m_pBuffer;
    uint32_t m_nSize;
};

#endif