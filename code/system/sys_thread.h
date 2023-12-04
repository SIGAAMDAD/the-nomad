#ifndef __SYS_THREAD__
#define __SYS_THREAD__

#pragma once

#ifdef _WIN32
#include <winnt.h>
#include <processthreadsapi.h>
#include <synchapi.h> //  For InitializeCriticalSection, etc.
#include <errhandlingapi.h> //  For GetLastError
#include <handleapi.h>
#elif defined(POSIX)
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#endif

#include "../engine/n_shared.h"

/*
template<typename T>
class CThreadAtomicInt
{
public:
	CThreadAtomicInt( void );
	~CThreadAtomicInt() { }
	
	void Store( const T& value );
	void Add( const T& value );
	void Subtract( const T& value );
	void Increment( const T& value );
	void Decrement( const T& value );
	void Store( const T& value );
	const T& Load( void ) const;
	LONG& Load( void );
private:
#ifndef USE_EASTL_ATOMIC
	uint64_t m_Value;
#else
	eastl::atomic<uint64_t> m_Value;
#endif
};

template<typename T>
GDR_INLINE CThreadAtomic<T>::CThreadAtomic( void )
	: m_Value( 0 )
{
}

template<typename T>
GDR_INLINE void CThreadAtomic<T>::Store( T value ) {
#ifdef USE_EASTL_ATOMIC
	m_Value.store( value );
#else
#ifdef _WIN32
	InterlockedExchange( (volatile LONG *)&m_Value, *(LONG *)&value );
#elif defined(POSIX)
#endif
#endif
}

template<typename T>
GDR_INLINE void CThreadAtomic<T>::Add( T value ) {
#ifdef USE_EASTL_ATOMIC
#else
#ifdef _WIN32
	InterlockedExchangeAdd( (volatile LONG *)&m_Value, *(LONG *)&value );
#else
	__atomic_();
#endif
#endif
}

*/

template<typename Mutex>
class CThreadAutoLock
{
public:
    GDR_INLINE CThreadAutoLock( Mutex& lock )
        : m_Lock( lock )
    {
        m_Lock.Lock();
    }
    GDR_INLINE ~CThreadAutoLock() {
        m_Lock.Unlock();
    }
private:
    Mutex& m_Lock;
};

class CThreadMutex
{
public:
	CThreadMutex( void );
	~CThreadMutex();
	
	void Lock( void );
	void Lock( void ) const;
	void Unlock( void );
	void Unlock( void ) const;
	
	bool TryLock( void );
	bool TryLock( void ) const;
private:
#ifdef _WIN32
//	SRWLOCK m_hLock;
	CRITICAL_SECTION m_hLock;
	DWORD m_OwnerThreadId;
	uint32_t m_nLockCount;
#elif defined(POSIX)
	pthread_mutex_t m_hLock;
	pthread_mutexattr_t m_hAttrib;
#endif
};

GDR_INLINE CThreadMutex::CThreadMutex( void )
{
#ifdef _WIN32
//	InitializeSRWLock( &m_hLock );
	InitializeCriticalSection( &m_hLock );
	m_OwnerThreadId = 0;
	m_nLockCount = 0;
#elif defined(POSIX)
	pthread_mutexattr_init( &m_hAttrib );
	pthread_mutexattr_settype( &m_hAttrib, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &m_hLock, &m_hAttrib );
#endif
}

GDR_INLINE CThreadMutex::~CThreadMutex()
{
#ifdef _WIN32
	DeleteCriticalSection( &m_hLock );
#elif defined(POSIX)
	pthread_mutexattr_destroy( &m_hAttrib );
	pthread_mutex_destroy( &m_hLock );
#endif
}

GDR_INLINE void CThreadMutex::Unlock( void )
{
#ifdef _WIN32
	if (m_OwnerThreadId == GetCurrentThreadId()) {
		m_nLockCount--;
		
		if (!m_nLockCount) {
			m_OwnerThreadId = 0;
			EnterCriticalSection( &m_hLock );
//			ReleaseSRWLockExclusive( &m_hLock );
		}
	}
#elif defined(POSIX)
    pthread_mutex_unlock( &m_hLock );
#endif
}

GDR_INLINE void CThreadMutex::Lock( void )
{
#ifdef _WIN32
	const DWORD currentThread = GetCurrentThreadId();
	if (m_OwnerThreadId == currentThread) {
		// already locked
		m_nLockCount++;
	}
	else {
//		AcquireSRWLockExclusive( &m_hLock );
		LeaveCriticalSection( &m_hLock );
		m_OwnerThreadId = currentThread;
		m_nLockCount = 1;
	}
#elif defined(POSIX)
	pthread_mutex_lock( &m_hLock );
#endif
}

/*
class CThreadSpinRWLock
{
public:
	CThreadSpinRWLock( void );
	~CThreadSpinRWLock();
	
	void ReadLock( void );
	void ReadUnlock( void );
	void WriteLock( void );
	void Lock( void );
	void Unlock( void );
	
private:
#ifdef _WIN32
	
#elif defined(POSIX)
	pthread_spinlock_t m_SpinLock;
	int64_t m_nReadCount;
#endif
};

GDR_INLINE void CThreadSpinRWLock::Unlock( void )
{
#ifdef _WIN32

#elif defined(POSIX)
	pthread_spin_unlock( &m_SpinLock );
#endif
}

GDR_INLINE void CThreadSpinRWLock::Lock( void )
{
#ifdef _WIN32
	
#elif defined(POSIX)
	while (1) {
		pthread_spin_lock( &m_SpinLock );
		if (m_nSpinCount == 0) {
			m_nSpinCount = -1;
			pthread_spin_unlock( &m_SpinLock );
			break;
		}
		pthread_spin_unlock( &m_SpinLock );
	}
#endif
}

GDR_INLINE void CThreadSpinRWLock::ReadUnlock( void )
{
#ifdef _WIN32

#elif defined(POSIX)
	m_nReaders--;
	pthread_spin_unlock( &m_hLock );
#endif
}

GDR_INLINE void CThreadSpinRWLock::ReadLock( void )
{
#ifdef _WIN32

#elif defined(POSIX)
	while (1) {
		pthread_spin_lock( &m_SpinLock );
		if (m_nSpinCount >= 0) {
			m_nSpinCount++;
			pthread_spin_unlock( &m_SpinLock );
			break;
		}
		pthread_spin_unlock( &m_SpinLock );
	}
#endif
}
*/

#if 0
class CThreadCondVar
{
public:
	CThreadCondVar( void );
	~CThreadCondVar();
	
	void Wait( CThreadMutex *pMutex );
	void TimedWait( CThreadMutex *pMutex, uint64_t nTimeout );
	void Signal( void );
	void Broadcast( void );
private:
#ifdef _WIN32
	CONDITION_VARIABLE m_hCondVar;
#elif defined(POSIX)
	pthread_cond_t m_hCondVar;
	pthread_condattr_t m_hAttrib;
#endif
};

GDR_INLINE CThreadCondVar::CThreadCondVar( void )
{
#ifdef _WIN32
	InitializeConditionVariable( &m_hCondVar );
#elif defined(POSIX)
	pthread_condattr_init( &m_hAttrib );
	pthread_cond_init( &m_hCondVar, &m_hAttrib );
#endif
}

GDR_INLINE CThreadCondVar::~CThreadCondVar()
{
#ifdef POSIX
	pthread_condattr_destroy( &m_hAttrib );
	pthread_cond_destroy( &m_hCondVar );
#endif
}

GDR_INLINE void CThreadCondVar::Signal( void )
{
#ifdef _WIN32
	WakeConditionVariable( &m_hCondVar );
#elif defined(POSIX)
	const int ret = pthread_cond_signal( &m_hCondVar );
#endif
}

class CThreadRWLock
{
public:
private:
	CThreadMutex m_Lock;
	CThreadCondVar m_ReadersDone;
	
	int64_t m_nReaders;
	int64_t m_nActiveWriters;
	int64_t m_nWaitingWriters;
};
#endif

class CThread
{
public:
	CThread( void );
	virtual ~CThread();
	
	bool IsAlive( void ) const;
	
	void SetName( const char *name );
	const char *GetName( void ) const;
	
	bool Join( uint64_t nTimeout = 0 );
	
	bool Terminate( int iExitCode );
	void Stop( int iExitCode );

	void Sleep( uint64_t nDuration );

	#ifdef Yield
	#undef Yield
	#endif
	void Yield( void );
	
	virtual bool Start( uint64_t nBytesStack = 0 );

#ifdef _WIN32
	friend uint64_t __stdcall ThreadProc( void *arg );
#else
	friend void *ThreadProc( void *arg );
#endif
protected:
	// optional init with extra stuff
	virtual bool Init( void ) = 0;
	
	// thread will run this, must be supplied by the derived class
	virtual int32_t Run( void ) = 0;
	
	// called when the thread exits
	virtual void OnExit( void );
	
#ifdef _WIN32
	typedef uint64_t (__stdcall *ThreadProc_t)( void *arg );
#else
	typedef void *(*ThreadProc_t)( void *arg );
#endif
	
	virtual ThreadProc_t GetThreadProc( void );
	CThreadMutex m_hLock;
private:
#ifdef _WIN32
	HANDLE m_hThread;
#elif defined(POSIX)
	pthread_t m_ThreadId;
	pthread_attr_t m_hAttrib;
#endif
	int32_t m_iResult;
	char m_szName[MAX_GDR_PATH];
};

class CWorkerThread : public CThread
{
public:
	CWorkerThread( void );
	virtual ~CWorkerThread() override;
private:
};

#include "sys_thread.inl"

#endif

// various win32 implementations of a mutex
/*
class CThreadMutex
{
public:
	CThreadMutex()
	{
		m_hMutex = CreateMutex( NULL, FALSE, NULL );
		m_iOwnerThreadId = 0;
		m_nLockCount = 0;
	}
	
	~CThreadMutex()
	{
		CloseHandle( m_hMutex );
	}
	
	void Lock( void )
	{
		const DWORD currentThreadId = GetCurrentThreadId();
		
		if (m_iOwnerThreadId == currentThreadId) {
			// already locked
			m_nLockCount++;
		}
		else {
			WaitForSingleObject( m_hMutex, INFINITE );
			m_iOwnerThreadId = currentThreadId;
			m_nLockCount = 1;
		}
	}
	
	void Unlock( void )
	{
		if (m_iOwnerThreadId == GetCurrentThreadId()) {
			m_nLockCount--;
			
			if (!m_nLockCount) {
				m_iOnwerThreadId = 0;
				ReleaseMutex( m_hMutex );
			}
		}
	}
private:
	HANDLE m_hMutex;
	DWORD m_iOwnerThreadId;
	uint32_t m_nLockCount;
};

class CThreadMutex
{
public:
	CThreadMutex( void )
	{
		InitializeSRWLock( &m_hLock );
		m_iOwnerThreadId = 0;
		m_nLockCount = 0;
	}
	
	~CThreadMutex()
	{}
	
	
	void Lock( void )
	{
		const DWORD currentThreadId = GetCurrentThreadId();
		
		if (m_iOwnerThreadId == currentThreadId) {
			// already locked
			m_nLockCount++;
		}
		else {
			AcquireSRWLockExclusive( &m_hLock );
			m_iOwnerThreadId = currentThreadId;
			m_nLockCount = 1;
		}
	}
	
	void Unlock( void )
	{
		if (m_iOwnerThreadId == GetCurrentThreadId()) {
			m_nLockCount--;
			
			if (!m_nLockCount) {
				m_iOnwerThreadId = 0;
				ReleaseSRWLockExclusive( &m_hLock );
			}
		}
	}
	
	
private:
	SRWLOCK m_hLock;
	DWORD m_iOwnerThreadId;
	uint32_t m_nLockCount;
};

class CThreadMutex
{
public:
	CThreadMutex( void )
	{
		InitializeCriticalSection( &m_hLock );
		m_iOwnerThreadId = 0;
		m_nLockCount = 0;
	}
	
	~CThreadMutex()
	{
		DeleteCriticalSection( &m_hLock );
	}
	
	
	void Lock( void )
	{
		const DWORD currentThreadId = GetCurrentThreadId();
		
		if (m_iOwnerThreadId == currentThreadId) {
			// already locked
			m_nLockCount++;
		}
		else {
			EnterCriticalSection( &m_hLock );
			m_iOwnerThreadId = currentThreadId;
			m_nLockCount = 1;
		}
	}
	
	void Unlock( void )
	{
		if (m_iOwnerThreadId == GetCurrentThreadId()) {
			m_nLockCount--;
			
			if (!m_nLockCount) {
				m_iOnwerThreadId = 0;
				LeaveCriticalSection( &m_hLock );
			}
		}
	}
	
	
private:
	CRITICAL_SECTION m_hLock;
	DWORD m_iOwnerThreadId;
	uint32_t m_nLockCount;
};
*/