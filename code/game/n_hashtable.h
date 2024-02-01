#ifndef __N_HASHTABLE__
#define __N_HASHTABLE__

#pragma once

template<typename T, typename Allocator>
class CHashList
{
public:
	CHashList( void );
	~CHashList();
	
	const CHashList<T, Allocator>& operator=( const CHashList<T, Allocator>& other );
	
	void Allocate( uint64_t nElems );
	void Clear( void );
	
	T *Hash( const char *pName );
	
	uint64_t Size( void ) const;
	uint64_t Allocated( void ) const;
	uint64_t HashSize( void ) const;
	
	const T *Get( const char *pName ) const;
	T *Get( const char *pName );
	
	const T *GetList( void ) const;
	T *GetList( void );
private:
	// disable copying
	CHashList( const CHashList & );
	CHashList( CHashList && );
	
	uint64_t HashSize( uint64_t hashCount ) const;
	uint64_t GetHashed( const char *pName ) const;

	typedef struct hashnode_s {
		T *pData;
		const char *pName;
		struct hashnode_s *next;
	} hashnode_t;
	
	Allocator m_Allocator;
	T *m_pBuildBuffer;
	hashnode_t *m_pHashTable;
	uint64_t m_nSize;
	uint64_t m_nAllocated;
	uint64_t m_nHashSize;
};

template<typename T, typename Allocator>
GDR_INLINE CHashList<T, Allocator>::CHashList( void )
{
	m_pBuildBuffer = NULL;
	m_pHashTable = NULL;
	m_nSize = 0;
	m_nAllocated = 0;
	m_nHashSize = 0;
}

template<typename T, typename Allocator>
GDR_INLINE CHashList<T, Allocator>::~CHashList()
{
	Clear();
}

template<typename T, typename Allocator>
GDR_INLINE void CHashList<T, Allocator>::Clear( void )
{
	uint64_t i;
	
	for ( i = 0; i < m_nSize; i++ ) {
		if ( m_pHashTable[i].pData ) {
			m_Allocator.deallocate( m_pHashTable[i].pData );
			m_pHashTable[i].pData = NULL;
		}
	}
	
	m_Allocator.deallocate( m_pBuildBuffer );
	m_pHashTable = NULL;
	m_nSize = 0;
	m_nAllocated = 0;
	m_nHashSize = 0;
}

template<typename T, typename Allocator>
GDR_INLINE void CHashList<T, Allocator>::Allocate( uint64_t nElems )
{
	Clear();
	m_nHashSize = HashSize( nElems );
	m_nSize = nElems;
	
	m_nAllocated = 0;
	m_nAllocated += PAD( sizeof(*m_pBuildBuffer) * nElems, sizeof(uintptr_t) );
	m_nAllocated += PAD( sizeof(*m_pHashTable) * m_nHashSize, sizeof(uintptr_t) );
	
	m_pBuildBuffer = (T *)m_Allocator.allocate( m_nAllocated );
	m_pHashTable = (hashnode_t *)( m_pBuildBuffer + nElems );
}

template<typename T, typename Allocator>
GDR_INLINE uint64_t CHashList<T, Allocator>::Size( void ) const
{
	return m_nSize;
}

template<typename T, typename Allocator>
GDR_INLINE uint64_t CHashList<T, Allocator>::Allocated( void ) const
{
	return m_nAllocated;
}

template<typename T, typename Allocator>
GDR_INLINE uint64_t CHashList<T, Allocator>::HashSize( void ) const
{
	return m_nHashSize;
}

template<typename T, typename Allocator>
GDR_INLINE const T *CHashList<T, Allocator>::Get( const char *pName ) const
{
	return m_pHashTable[GetHashed( pName )].pData;
}

template<typename T, typename Allocator>
GDR_INLINE T *CHashList<T, Allocator>::Get( const char *pName )
{
	return m_pHashTable[GetHashed( pName )].pData;
}

template<typename T, typename Allocator>
GDR_INLINE const T *CHashList<T, Allocator>::GetList( void ) const
{
	return m_pBuildBuffer;
}

template<typename T, typename Allocator>
GDR_INLINE T *CHashList<T, Allocator>::GetList( void )
{
	return m_pBuildBuffer;
}

template<typename T, typename Allocator>
GDR_INLINE uint64_t CHashList<T, Allocator>::GetHashed( const char *pName ) const
{
	const hashnode_t *node;
	uint64_t hash;

	hash = Com_GenerateHashValue( pName, m_nHashSize );
	for ( node = m_pHashTable[hash]; node; node = node->next ) {
		if ( !N_stricmp( node->pName, pName ) ) {
			return (uint64_t)(uintptr_t);
		}
	}
}

template<typename T, typename Allocator>
GDR_INLINE void CHashList<T, Allocator>::Remove( const char *pName )
{
	hashnode_t *node;
	
	node = &m_pHashTable[GetHashed( pName )];
	if ( !node->pData ) {
		return;
	}
	
	memset( node->pData, 0, sizeof(T) );
	node->next = NULL;
	node->pName = NULL;
	m_nSize--;
}

template<typename T, typename Allocator>
GDR_INLINE T *CHashList<T, Allocator>::Hash( const char *pName )
{
	T *elem;
	hashnode_t *node;
	uint64_t hash, size;
	
	hash = GetHashed( pName );
	elem = &m_pBuildBuffer[m_nSize];
	
	node->pData = elem;
	node->next = m_pHashTable[hash];
	node->pName = pName;
	m_pHashTable[hash] = node;
	
	m_nSize++;
	
	return node->m_pData;
}

#define MAX_HASH_SIZE 2048

template<typename T, typename Allocator>
uint64_t CHashList<T, Allocator>::HashSize( uint64_t hashCount ) const
{
	uint64_t hashSize;

	for ( hashSize = 2; hashSize < MAX_HASH_SIZE; hashSize <<= 1 ) {
		if ( hashSize >= hashCount ) {
			break;
		}
	}

	return hashSize;
}

#endif