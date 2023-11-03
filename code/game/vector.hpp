#ifndef __VECTOR__
#define __VECTOR__

#pragma once

template<typename T, int tag = TAG_STATIC>
class CVector
{
public:
	CVector(void);
	~CVector();

	void Init( void );
	void Shutdown( void );
	
	void Reserve( uint64_t amount );
	void Resize( uint64_t size );
	void Clear( void );

	const T& Insert( T&& thingy );
	const T& Insert( const T& thingy );

	uint64_t Size( void ) const;
	uint64_t Allocated( void ) const;
	uint64_t BytesUsed( void ) const;
private:
	T *mData;
	uint64_t mSize;
	uint64_t mAllocated;

	void doRealloc( uint64_t addSize );
};

template<typename T, int tag>
void CVector<T, tag>::doRealloc( uint64_t addSize )
{
	T *tmp;

	mAllocated += addSize << 1;
	tmp = (T *)Z_Malloc(mAllocated * sizeof(T), tag);
	if (mData) {
		memcpy(mData, tmp, mSize);
		Z_Free(mData);
	}
	mData = tmp;
}

template<typename T, int tag>
void CVector<T, tag>::Reserve( uint64_t amount )
{
	T *tmp;

	mAllocated += amount;
	tmp = (T *)Z_Malloc(mAllocated * sizeof(T), tag);
	if (mData) {
		memcpy(mData, tmp, mSize);
		Z_Free(mData);
	}
	mData = tmp;
}

template<typename T, int tag>
void CVector<T, tag>::Resize( uint64_t size )
{
	T *tmp;

	mSize = size;
	mAllocated = size;

	tmp = (T *)Z_Malloc(mAllocated * sizeof(T), tag);
	if (mData) {
		memcpy(tmp, mData, mSize);
		Z_Free(mData);
	}
	mData = tmp;
}

#endif