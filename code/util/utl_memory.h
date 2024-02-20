#ifndef __UTL_MEMORY__
#define __UTL_MEMORY__

#pragma once

#include <stdint.h>
#include <string.h>

template<typename T>
class CUtlMemory
{
public:
	CUtlMemory( T *pBuffer = NULL, uint64_t nItems = 0 );
	CUtlMemory( const CUtlMemory& other );
	~CUtlMemory();

	// returns the number of items used
	uint64_t Size( void ) const;

	// returns sizeof(T) * Size()
	uint64_t BytesUsed( void ) const;

	// returns the total bytes allocated in the buffer
	uint64_t Allocated( void ) const;

	bool IsExternal( void ) const;

	void SetGrowSize( uint64_t nGrowSize = 2 );

	void Grow( uint64_t nItems );
private:
	CUtlMemory( CUtlMemory&& other );

	void *m_pBuffer;
	uint64_t m_nSize;
};

#endif