#include "module_public.h"
#include "module_link.hpp"

extern "C" CModuleDataLink::CModuleDataLink( void ) {
}

extern "C" CModuleDataLink::CModuleDataLink( uint64_t nBytes )
{
    Resize( nBytes );
}

extern "C" CModuleDataLink::~CModuleDataLink()
{
    Release();
}

extern "C" uint64_t CModuleDataLink::Size( void ) const
{
    return m_nBytes;
}

extern "C" void *CModuleDataLink::GetBuffer( void )
{
    return m_pMemory;
}

extern "C" const void *CModuleDataLink::GetBuffer( void ) const
{
    return m_pMemory;
}

extern "C" void CModuleDataLink::Create( vm_t *pModule0, vm_t *pModule1 )
{
}

extern "C" void CModuleDataLink::Release( void )
{
    m_hLock.WriteLock();

    if ( m_pMemory ) {
        delete m_pMemory;
    }

    m_pMemory = NULL;
    m_nBytes = 0;

    m_hLock.WriteUnlock();
}

extern "C" void CModuleDataLink::Resize( uint64_t nBytes )
{
    if ( nBytes <= m_nBytes ) {
        return;
    }

    Release();
    m_hLock.WriteLock();
    m_nBytes = nBytes;
    m_pMemory = new char[m_nBytes];
    m_hLock.WriteUnlock();
}

extern "C" void CModuleDataLink::Read( void *pBuffer, uint32_t nBytes ) const
{
    if ( !nBytes || nBytes > m_nBytes ) {
        return;
    }

    m_hLock.ReadLock();
    memcpy( pBuffer, m_pMemory, nBytes );
    m_hLock.ReadUnlock();
}

extern "C" void CModuleDataLink::Write( const void *pBuffer, uint32_t nBytes )
{
}
