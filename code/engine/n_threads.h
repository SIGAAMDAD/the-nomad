#ifndef __N_THREADS__
#define __N_THREADS__

#pragma once

#if defined(_WIN32)
	typedef void *HANDLE;
	
#elif defined(__unix__)
	#include <pthread.h>
	#include <errno.h>
	#include <signal.h>
#endif

#ifdef _MSC_VER
	#include <intrin.h>
#endif

typedef uintptr_t ThreadHandle_t;
typedef uintptr_t (*ThreadFunc_t)( void *pArgs );

#define TT_INFINITE 0xffffffff

#ifdef GDRx64
typedef uint64_t ThreadId_t;
#else
typedef uint32_t ThreadId_t;
#endif

GDR_INLINE void ThreadPause( void )
{
#if defined( COMPILER_PS3 )
	__db16cyc();
#elif defined( __GNUC__ )
	__asm __volatile( "pause" );
#elif defined ( _MSC_VER ) && defined(_WIN64)
	_mm_pause();
#elif defined( _MSC_VER ) && defined(_WIN32)
	__asm pause;
//#elif defined( COMPILER_MSVCX360 )
//	YieldProcessor(); 
//	__asm { or r0,r0,r0 } 
//	YieldProcessor(); 
//	__asm { or r1,r1,r1 } 
#else
#error "implement me"
#endif
}

#if __cplusplus < 201103L
	#ifdef __GNUC__
		#define THREAD_LOCAL __thread
	#elif defined(_MSC_VER)
		#define THREAD_LOCAL __declspec(thread)
	#else
		#error "need a thread_local"
	#endif
#else
	#define THREAD_LOCAL thread_local
#endif

class CThreadLocalData
{
public:
	CThreadLocalData( void );
	CThreadLocalData( void *pData, uint64_t nBytes );
	CThreadLocalData( uint64_t nBytes );
	~CThreadLocalData();
	
	void *GetData( void );
	const void *GetData( void ) const;
	uint64_t GetSize( void ) const;
	
	uint64_t Flush( void );
	void SetData( void *pData, uint64_t nBytes );
protected:
	void *Allocate( uint64_t nBytes );
	void Release( void *pBuffer );
	
	void AllocTLS( void );
	
#ifdef _WIN32
	uint32_t m_nIndex;
#else
	pthread_key_t m_hDataKey;
#endif
	void *m_pData;
	uint64_t m_nBytes;
};

template<typename Mutex>
class CThreadAutoLock
{
public:
	GDR_INLINE CThreadAutoLock( Mutex& hMutex )
		: m_hLock( hMutex )
	{
		Lock();
	}
	GDR_INLINE ~CThreadAutoLock()
	{
		Unlock();
	}

	GDR_INLINE void Lock( void ) {
		m_hLock.Lock();
	}
	GDR_INLINE void Unlock( void ) {
		m_hLock.Unlock();
	}
private:
	Mutex& m_hLock;
};

class CThreadMutex
{
public:
	CThreadMutex( void );
	~CThreadMutex();
	
	// Use this to make deadlocks easier to track by asserting
	// when it is excpected that the current thread owns the mutex
	bool AssertOwnedByCurrentThread( void ) const;
	
	void Lock( void );
	void Unlock( void );
	bool TryLock( void );
	GDR_INLINE void Lock( void ) const { const_cast<CThreadMutex *>( this )->Lock(); }
	GDR_INLINE void Unlock( void ) const { const_cast<CThreadMutex *>( this )->Unlock(); }
	GDR_INLINE bool TryLock( void ) const { return const_cast<CThreadMutex *>( this )->TryLock(); }
private:
#ifdef _WIN32
	SRWLOCK m_hLock;
	DWORD m_hOwnerThread;
#else
	pthread_mutex_t m_hMutex;
	pthread_mutexattr_t m_hAttrib;
	pthread_t m_hOwnerThread;
#endif
	int32_t m_nLockCount;
};

class CThreadFastMutex
{
public:
	CThreadFastMutex( void );
	~CThreadFastMutex();
	
	bool TryLock( uint32_t threadId ) volatile;
	void Lock( uint32_t nSpinSleepTime = 0 ) volatile;
	void Unlock( void ) volatile;
	bool TryLock( void ) volatile;
	
	bool TryLock( void ) const volatile;
	void Lock( unsigned nSpinSleepTime = 0 ) const volatile;
	void Unlock( void ) const volatile;
	
	// to match regular CThreadMutex
	bool AssertOwnedByCurrentThread( void ) const { return true; }
	
	uint32_t GetOwnerID( void ) const {
		return m_nOwnerID;
	}
	int32_t GetDepth( void ) const {
		return m_nDepth;
	}
	
private:
	bool TryLockInternal( uint32_t threadId ) volatile;
	
	volatile uint32_t m_nOwnerID;
	int32_t m_nDepth;
#ifndef _WIN32
	pthread_barrier_t m_MemoryBarrier;
#endif
};

template<typename Mutex>
class CAutoLock
{
public:
    GDR_INLINE CAutoLock( Mutex& mutex )
        : m_hLock( mutex )
    {
        Lock();
    }
	GDR_INLINE CAutoLock( const Mutex& mutex )
		: m_hLock( const_cast<Mutex&>( mutex ) )
	{
	}
    GDR_INLINE ~CAutoLock()
    {
        Unlock();
    }

    GDR_INLINE void Lock( void )
    {
        m_hLock.Lock();
    }

    GDR_INLINE void Unlock( void )
    {
        m_hLock.Unlock();
    }
private:
    Mutex& m_hLock;
};

class CThreadSyncObject
{
public:
	~CThreadSyncObject();
	
	// query if object is useful
	bool operator!( void ) const;
	
	// access handle
#ifdef _WIN32
	operator HANDLE( void ) { return GetHandle(); }
	inline const HANDLE GetHandle( void ) const { return m_hSyncObject; }
#endif
	
	// wait for a signal from the object
	bool Wait( uint32_t nTimeout = TT_INFINITE );
	
	//
	// wait for a signal from any of the specified objects.
	//
	// returns the index of the object that signaled the event
	// or THREADSYNC_TIMEOUT if the timeout was hit ebfore the wait confition was met.
	//
	// returns TW_FAILED if an incoming object is invalid
	//
	// if bWaitAll == true, then it'll return 0 if all the objects were set
	//
	static uint32_t WaitForMultiple( uint32_t nObjects, CThreadSyncObject **pObjects, bool bWaitAll, uint32_t nTimeout = TT_INFINITE );
	
protected:
	CThreadSyncObject( void );
	
	void AssertUseable( void ) const;
	
#ifdef _WIN32

#else
	pthread_mutex_t m_hMutex;
	pthread_cond_t m_hCondition;
	bool m_bInitialized;
#endif
};

class CThreadRWMutex
{
public:
	CThreadRWMutex( void );
	~CThreadRWMutex();
	
	void ReadLock( void );
	void WriteLock( void );
	void ReadUnlock( void );
	void WriteUnlock( void );
	
	GDR_INLINE void ReadLock( void ) const { const_cast<CThreadRWMutex *>( this )->ReadLock(); }
	GDR_INLINE void WriteLock( void ) const { const_cast<CThreadRWMutex *>( this )->WriteLock(); }
	GDR_INLINE void ReadUnlock( void ) const { const_cast<CThreadRWMutex *>( this )->ReadUnlock(); }
	GDR_INLINE void WriteUnlock( void ) const { const_cast<CThreadRWMutex *>( this )->WriteUnlock(); }
private:
#ifdef _WIN32
	SRWLOCK m_hLock;
	CONDITION_VARIABLE m_hCondition;
#else
	pthread_rwlock_t m_hRWLock;
	pthread_mutex_t m_hMutex;
	pthread_cond_t m_hCondition;
#endif
	uint32_t m_nReaders;
	uint32_t m_nActiveWriters;
	uint32_t m_nWaitingWriters;
};

typedef enum : uint32_t {
#ifdef _WIN32
	MemoryOrder_Relaxed,
	MemoryOrder_Consume,
	MemoryOrder_Acquire,
	MemoryOrder_Release,
	MemoryOrder_AcqRel,
	MemoryOrder_SeqCst
#else
	MemoryOrder_Relaxed = __ATOMIC_RELAXED,
	MemoryOrder_Consume = __ATOMIC_CONSUME,
	MemoryOrder_Acquire = __ATOMIC_ACQUIRE,
	MemoryOrder_Release = __ATOMIC_RELEASE,
	MemoryOrder_AcqRel = __ATOMIC_ACQ_REL,
	MemoryOrder_SeqCst = __ATOMIC_SEQ_CST
#endif
} MemoryOrder;

template<typename T>
class CThreadAtomic
{
public:
	CThreadAtomic( void );
	GDR_INLINE explicit CThreadAtomic( const T& value )
	{ store( m_hValue ); }
	~CThreadAtomic();
	
	const CThreadAtomic<T>& operator++( void );
	const CThreadAtomic<T>& operator++( int );
	
	const CThreadAtomic<T>& operator--( void );
	const CThreadAtomic<T>& operator--( int );
	
	const CThreadAtomic<T>& operator+=( const T& value );
	const CThreadAtomic<T>& operator-=( const T& value );
	const CThreadAtomic<T>& operator*=( const T& value );
	const CThreadAtomic<T>& operator/=( const T& value );
	
	CThreadAtomic<T> operator+( const T& rhs ) const;
	CThreadAtomic<T> operator-( const T& rhs ) const;
	
	const CThreadAtomic<T>& operator=( const T& value );
	
	const T& operator()( void ) const;
	operator T( void ) const;
	
	bool operator!( void ) const;
	bool operator!=( const T& value ) const;
	bool operator==( const T& value ) const;
	
	const T& load( MemoryOrder order = MemoryOrder_SeqCst ) const;
	T& load( MemoryOrder order = MemoryOrder_SeqCst );
	void store( T value, MemoryOrder order = MemoryOrder_SeqCst );
	void add( T value, MemoryOrder order = MemoryOrder_SeqCst );
	void sub( T value, MemoryOrder order = MemoryOrder_SeqCst );
	void fetch_add( MemoryOrder order = MemoryOrder_SeqCst );
	void fetch_sub( MemoryOrder order = MemoryOrder_SeqCst );
private:
	volatile T m_hValue;
};

class CThread
{
public:
	CThread( void );
	virtual ~CThread();
	
	// -----------------------------------------------

	const char *GetName( void );
	void SetName( const char *pName );
	
	size_t CalcStackDepth( void *pStackAddress );
	
	void SetStack( void *pStack, uint64_t nBytes );

	// returns true if the thread has been created and hasn't yet exited
	bool IsAlive( void );

	// this method causes the current thread to wait until this thread
	// is no longer alive
	bool Join( uint64_t nTimeout = TT_INFINITE );

	// access the thread handle directly
	ThreadHandle_t GetThreadHandle( void );

	int32_t GetResult( void );

	// forcibly, abnormally, but relatively cleanly stop the thread
	void Stop( int exitCode = 0 );

	// force hard-termination of thread, used only for critical failures
	bool Terminate( int exitCode = 0 );

	virtual bool Start( uint64_t nBytesStack = 0 );

	// ---------------------------
	// Global methods
	// ---------------------------

	// Get the thread object that represents the current thread, if any.
	// Can return NULL if the current thread wasn't created using CThread
	static CThread *GetCurrentCThread( void );

	// Offer a context switch. Under Win32, equivalent to Sleep(0)
#ifdef Yield
#undef Yield
#endif
	static void Yield( void );

	// This method causes the current thread to yield and not to be
	// scheduled for further execution until a certain amount of real
	// time has elapsed, more or less. Duration is in milliseconds
	static void Sleep( uint64_t duration );
protected:
	enum Flags
	{
		SUPPORT_STOP_PROTOCOL = 1 << 0
	};

	// Optional pre-run call, with ability to fail-create. Note Init()
	// is forced synchronous with Start()
	virtual bool Init( void );

	// Thread will run this function on startup, must be supplied by
	// derived class, performs the intended action of the thread.
	virtual int Run( void );

	// Called when the thread exits
	virtual void OnExit( void );

	// Allow for custom start waiting
//	virtual bool WaitForCreateComplete( CThreadEvent *pEvent );
	const ThreadId_t GetThreadID( void ) const { return (ThreadId_t)m_hThreadId; }

#ifdef _WIN32
	const ThreadHandle_t GetThreadHandle() const { return (ThreadHandle_t)m_hThread; }

	static unsigned long __stdcall ThreadProc( void * pv );
	typedef unsigned long (__stdcall *ThreadProc_t)( void * );
#else
	static void* ThreadProc( void * pv );
	typedef void* (*ThreadProc_t)( void * pv );
#endif

	virtual ThreadProc_t GetThreadProc( void );
	CThreadMutex m_hLock;
//	CThreadEvent m_ExitEvent;	// Set right before the thread's function exits.

private:
	// Thread initially runs this. param is actually 'this'. function
	// just gets this and calls ThreadProc
	struct ThreadInit_t
	{
		CThread *     pThread;
		bool *        pfInitSuccess;
#if defined( THREAD_PARENT_STACK_TRACE_ENABLED )
		void *        ParentStackTrace[THREAD_PARENT_STACK_TRACE_LENGTH];
#endif
	};

	// make copy constructor and assignment operator inaccessible
	CThread( const CThread & );
	CThread &operator=( const CThread & );

#ifdef _WIN32
	HANDLE 	m_hThread;
	ThreadId_t m_ThreadId;
#else
	pthread_t m_hThreadId;
	pthread_attr_t m_ThreadAttribs;
	volatile pthread_t m_hThreadZombieId;
#endif
	int32_t	m_Result;
	char	m_szName[MAX_NPATH];
	void	*m_pStackBase;
	uint32_t m_Flags;
	ThreadInit_t *m_pThreadInit;
};

inline CThread *g_pCurThread;

GDR_INLINE CThread::CThread( void ) :
#ifdef _WIN32
	m_hThread( NULL ), m_ThreadId( 0 ),
#else
	m_hThreadId( 0 ), m_hThreadZombieId( 0 ),
#endif
	m_Result( 0 ), m_Flags( 0 )
{
#ifndef _WIN32
	pthread_attr_init( &m_ThreadAttribs );
#endif
	memset( m_szName, 0, sizeof(m_szName) );
	m_pThreadInit = (ThreadInit_t *)Z_Malloc( sizeof(*m_pThreadInit), TAG_STATIC );
}

GDR_INLINE CThread::~CThread()
{
#ifdef _MSC_VER
	if ( m_hThread )
#else
	if ( m_hThreadId )
#endif
	{
		if ( IsAlive() ) {
			Con_Printf( COLOR_RED "WARNING: Illegal termination of worker thread! Threads must negotiate an end to the thread before the CThread object is destroyed.\n" );
			Sys_MessageBox( va( __FILE__ ":%u", __LINE__ ),
				"Illegal termination of worker thread! Threads must negotiate an end to the thread before the CThread object is destroyed.", false );
		}
	}

#ifdef __unix__
	if ( m_hThreadZombieId ) {
		// just clean up zombie threads immediately (the destructor is fired from the hosting thread)
		Join();
	}
#endif
	if ( m_pThreadInit ) {
		Z_Free( m_pThreadInit );
		m_pThreadInit = NULL;
	}
}

GDR_INLINE const char *CThread::GetName( void )
{
	CThreadAutoLock<CThreadMutex> lock( m_hLock );
	if ( !m_szName[0] ) {
		memset( m_szName, 0, sizeof(m_szName) );
	#ifdef _WIN32
		_snprintf( m_szName, sizeof(m_szName) - 1, "Thread(%p/%p)", this, m_hThread );
	#else
		snprintf( m_szName, sizeof(m_szName) - 1, "Thread(%p/0x%04x)", this, (uint32_t)m_hThreadId );
	#endif
	}
	return m_szName;
}

GDR_INLINE void CThread::SetName( const char *pName )
{
	CThreadAutoLock<CThreadMutex> lock( m_hLock );
	N_strncpyz( m_szName, pName, sizeof(m_szName) );
}


//-----------------------------------------------------
// Functions for the other threads
//-----------------------------------------------------

// Start thread running  - error if already running
GDR_INLINE bool CThread::Start( uint64_t nBytesStack )
{
	CThreadAutoLock<CThreadMutex> lock( m_hLock );

	if ( IsAlive() ) {
		AssertMsg( 0, "Tried to create a thread that has already been created!" );
		return false;
	}

	bool bInitSuccess = false;
	m_pThreadInit->pThread = this;
	m_pThreadInit->pfInitSuccess = &bInitSuccess;

#if defined( THREAD_PARENT_STACK_TRACE_ENABLED )
	{
		int iValidEntries = GetCallStack_Fast( init.ParentStackTrace, arraylen( init.ParentStackTrace ), 0 );
		for( int i = iValidEntries; i < arraylen( init.ParentStackTrace ); ++i )
		{
			init.ParentStackTrace[i] = NULL;
		}
	}
#endif

#ifdef _WIN32
	m_hThread = (HANDLE)CreateThread( NULL,
		nBytesStack,
		(LPTHREAD_START_ROUTINE)GetThreadProc(),
		m_pThreadInit,
		0,
		(LPDWORD)&m_ThreadId );

	if ( !m_hThread ) {
		AssertMsg1( 0, "Failed to create thread (error 0x%x)", GetLastError() );
		return false;
	}
#else
	pthread_attr_setstacksize( &m_ThreadAttribs, MAX( nBytesStack, 1024u*1024u ) );
	if ( pthread_create( &m_hThreadId, &m_ThreadAttribs, (void *(*)(void *))GetThreadProc(), m_pThreadInit ) != 0 )
	{
		AssertMsg1( 0, "Failed to create thread (error 0x%x)", errno );
		return false;
	}
	bInitSuccess = true;
#endif


/*
	if ( !WaitForCreateComplete( &createComplete ) ) {
		Con_Printf( "Thread failed to initialize\n" );
#ifdef _WIN32
		CloseHandle( m_hThread );
		m_hThread = NULL;
#endif
		return false;
	}
*/

	if ( !bInitSuccess ) {
		Con_Printf( "Thread failed to initialize\n" );
#ifdef _WIN32
		CloseHandle( m_hThread );
		m_hThread = NULL;
#else
		m_hThreadId = 0;
		m_hThreadZombieId = 0;
#endif
		return false;
	}

#ifdef _WIN32
	if ( !m_hThread ) {
		Con_Printf( "Thread exited immediately\n" );
	}
#endif

#ifdef _WIN32
	AddThreadHandleToIDMap( m_hThread, m_ThreadId );
	return !!m_hThread;
#else
	return !!m_hThreadId;
#endif
}

GDR_INLINE bool CThread::IsAlive( void )
{
#ifdef _WIN32
	DWORD dwExitCode;
	return (
		m_hThread 
		&& GetExitCodeThread( m_hThread, &dwExitCode ) 
		&& dwExitCode == STILL_ACTIVE );
#else
	return !!m_hThreadId;
#endif
}

//-----------------------------------------------------------------------------
GDR_INLINE bool ThreadJoin( ThreadHandle_t hThread, uint64_t timeout )
{
	if ( !hThread ) {
		return false;
	}

#ifdef _WIN32
	DWORD dwWait = WaitForSingleObject( (HANDLE)hThread, timeout );
	if ( dwWait == WAIT_TIMEOUT ) {
		return false;
	}
	if ( dwWait != WAIT_OBJECT_0 && ( dwWait != WAIT_FAILED && GetLastError() != 0 ) ) {
		Assert( 0 );
		return false;
	}
#elif defined(POSIX)
	if ( pthread_join( (pthread_t)hThread, NULL ) != 0 ) {
		return false;
	}
#else
	Assert( 0 );
	DebuggerBreak();
#endif
	return true;
}


// This method causes the current thread to wait until this thread
// is no longer alive.
GDR_INLINE bool CThread::Join( uint64_t timeout )
{
#ifdef _WIN32
	if ( m_hThread )
#else
	if ( m_hThreadId || m_hThreadZombieId )
#endif
	{
		AssertMsg( GetCurrentCThread() != this, "Thread cannot be joined with self" );

#ifdef _WIN32
		return ThreadJoin( (ThreadHandle_t)m_hThread, timeout );
#else
		bool ret = ThreadJoin(  (ThreadHandle_t)( m_hThreadId ? m_hThreadId : m_hThreadZombieId), timeout );
		m_hThreadZombieId = 0;
		return ret;
#endif
	}
	return true;
}

//---------------------------------------------------------

GDR_INLINE ThreadHandle_t CThread::GetThreadHandle( void )
{
#ifdef _WIN32
	return (ThreadHandle_t)m_hThread;
#else
	return (ThreadHandle_t)m_hThreadId;
#endif
}


//---------------------------------------------------------

GDR_INLINE int32_t CThread::GetResult( void )
{
	return m_Result;
}

//-----------------------------------------------------
// Functions for both this, and maybe, and other threads
//-----------------------------------------------------

// Forcibly, abnormally, but relatively cleanly stop the thread
//

GDR_INLINE void CThread::Stop( int exitCode )
{
	if ( !IsAlive() ) {
		return;
	}

	if ( GetCurrentCThread() == this ) {
		m_Result = exitCode;
		if ( !( m_Flags & SUPPORT_STOP_PROTOCOL ) ) {
			OnExit();
			g_pCurThread = NULL;

#ifdef _WIN32
			CloseHandle( m_hThread );
			RemoveThreadHandleToIDMap( m_hThread );
			m_hThread = NULL;
#else
			m_hThreadId = 0;
			m_hThreadZombieId = 0;
#endif
		}
		else {
			throw exitCode;
		}
		AssertMsg( false, "Called CThread::Stop() for a platform that doesn't have it!\n");
	}
	else {
		AssertMsg( 0, "Only thread can stop self: Use a higher-level protocol");
	}
}

GDR_INLINE size_t CThread::CalcStackDepth( void *pStackAddress )
{
	return ( (byte *)m_pStackBase - (byte *)pStackAddress );
}

GDR_INLINE void CThread::SetStack( void *pStack, uint64_t nBytes )
{
	m_pStackBase = pStack;
#ifdef _WIN32

#else
	pthread_attr_setstack( &m_ThreadAttribs, pStack, nBytes );
	pthread_attr_setguardsize( &m_ThreadAttribs, nBytes );
#endif
}

//---------------------------------------------------------

// Force hard-termination of thread, used only for critical failures.
GDR_INLINE bool CThread::Terminate( int exitCode )
{
#ifdef _WIN32
	// I hope you know what you're doing!
	if ( !TerminateThread( m_hThread, exitCode ) ) {
		return false;
	}
	CloseHandle( m_hThread );
	RemoveThreadHandleToIDMap( m_hThread );
	m_hThread = NULL;
#else
	pthread_kill( m_hThreadId, SIGKILL );
	m_hThreadId = 0;
#endif
	return true;
}


//-----------------------------------------------------
// Global methods
//-----------------------------------------------------

// Get the Thread object that represents the current thread, if any.
// Can return NULL if the current thread was not created using
// CThread
//

GDR_INLINE CThread *CThread::GetCurrentCThread( void )
{
	return g_pCurThread;
}

//---------------------------------------------------------
//
// Offer a context switch. Under Win32, equivalent to Sleep(0)
//

#ifdef Yield
#undef Yield
#endif
GDR_INLINE void CThread::Yield( void )
{
#ifdef _WIN32
	::Sleep( 0 );
#elif defined(POSIX)
	sched_yield();
#endif
}

//---------------------------------------------------------
//
// This method causes the current thread to yield and not to be
// scheduled for further execution until a certain amount of real
// time has elapsed, more or less. Duration is in milliseconds

GDR_INLINE void CThread::Sleep( uint64_t duration )
{
#ifdef _WIN32
	::Sleep( duration );
#elif defined(POSIX)
	usleep( duration * 1000 );
#endif
}

//---------------------------------------------------------

// Optional pre-run call, with ability to fail-create. Note Init()
// is forced synchronous with Start()
GDR_INLINE bool CThread::Init( void )
{
	return true;
}

//---------------------------------------------------------

GDR_INLINE int32_t CThread::Run( void )
{
	return -1;
}

// Called when the thread exits
GDR_INLINE void CThread::OnExit( void ) { }

/*
// Allow for custom start waiting
GDR_INLINE bool CThread::WaitForCreateComplete( CThreadEvent *pEvent )
{
	// Force serialized thread creation...
	if (!pEvent->Wait(60000))
	{
		AssertMsg( 0, "Probably deadlock or failure waiting for thread to initialize." );
		return false;
	}
	return true;
}
*/

//---------------------------------------------------------
GDR_INLINE CThread::ThreadProc_t CThread::GetThreadProc( void )
{
	return ThreadProc;
}

#ifdef _WIN32
GDR_INLINE uint64_t __stdcall CThread::ThreadProc( LPVOID pv )
#else
GDR_INLINE void *CThread::ThreadProc( void *pv )
#endif
{
	ThreadInit_t *pInit = (ThreadInit_t *)pv;

	CThread *pThread = pInit->pThread;
	g_pCurThread = pThread;

	pThread->m_pStackBase = (void *)PAD( (uintptr_t)&pThread, 4096 );

	pInit->pThread->m_Result = -1;

#if defined( THREAD_PARENT_STACK_TRACE_ENABLED )
	CStackTop_ReferenceParentStack stackTop( pInit->ParentStackTrace, arraylen( pInit->ParentStackTrace ) );
#endif

	bool bInitSuccess = true;
	if ( pInit->pfInitSuccess ) {
		*( pInit->pfInitSuccess ) = false;
	}

	try {
		bInitSuccess = pInit->pThread->Init();
	} catch (...) {
//		pInit->pInitCompleteEvent->Set();
		throw;
	}

	if ( pInit->pfInitSuccess ) {
		*( pInit->pfInitSuccess ) = bInitSuccess;
	}

//	pInit->pInitCompleteEvent->Set();
	if ( !bInitSuccess ) {
		return 0;
	}

	if ( !Sys_IsInDebugSession() && ( pInit->pThread->m_Flags & SUPPORT_STOP_PROTOCOL ) ) {
		try {
			pInit->pThread->m_Result = pInit->pThread->Run();
		} catch (...) {
		}
	}
	else {
		pInit->pThread->m_Result = pInit->pThread->Run();
	}

	pInit->pThread->OnExit();
	g_pCurThread = NULL;

	CThreadAutoLock<CThreadMutex> lock( pThread->m_hLock );
#ifdef _WIN32
	CloseHandle( pThread->m_hThread );
	RemoveThreadHandleToIDMap( pThread->m_hThread );
	pThread->m_hThread = NULL;
#elif defined(POSIX)
	pThread->m_hThreadZombieId = pThread->m_hThreadId;
	pThread->m_hThreadId = 0;
#else
#error "implement me"
#endif

//	pThread->m_ExitEvent.Set();

#if defined( POSIX )
	return (void *)(uintptr_t)pInit->pThread->m_Result;
#else
	return pInit->pThread->m_Result;
#endif
}

GDR_INLINE CThreadLocalData::CThreadLocalData( void *pData, uint64_t nBytes )
	: m_pData{ pData }, m_nBytes{ nBytes }
{
	SetData( pData, nBytes );
}

GDR_INLINE CThreadLocalData::CThreadLocalData( void )
{
	m_pData = NULL;
	m_nBytes = 0;
}

GDR_INLINE CThreadLocalData::~CThreadLocalData()
{
#ifdef _WIN32
	if ( m_pData && m_nBytes ) {
		TlsFree( m_nIndex );
	}
#else
	if ( m_pData && m_nBytes ) {
		pthread_key_delete( m_hDataKey );
		Release( m_pData );
	}
#endif
}

GDR_INLINE void *CThreadLocalData::GetData( void )
{
	return m_pData;
}

GDR_INLINE const void *CThreadLocalData::GetData( void ) const
{
	return m_pData;
}

GDR_INLINE uint64_t CThreadLocalData::GetSize( void ) const
{
	return m_nBytes;
}

GDR_INLINE void CThreadLocalData::AllocTLS( void )
{
#ifdef _WIN32
	m_nIndex = TlsAlloc();
	if ( m_nIndex == TLS_OUT_OF_INDEXES ) {
		N_Error( ERR_DROP, "CThreadLocalData::AllocTLS: TLS_OUT_OF_INDEXES, %04x0", GetLastError() );
	}
#else
	pthread_key_create( &m_hDataKey, NULL );
#endif
}

GDR_INLINE void *CThreadLocalData::Allocate( uint64_t nBytes )
{
	// ensure to allocate a fretch bunch
	Release( m_pData );
	
	return ( m_pData = Z_Malloc( nBytes, TAG_STATIC ) );
}

GDR_INLINE void CThreadLocalData::Release( void *pBuffer )
{
	if ( pBuffer ) {
		Z_Free( pBuffer );
	}
	m_pData = NULL;
	m_nBytes = 0;
}

GDR_INLINE void CThreadLocalData::SetData( void *pData, uint64_t nBytes )
{
	if ( !pData || !nBytes ) {
#ifdef _WIN32
		TlsFree( m_nIndex );
#else
		pthread_key_delete( m_hDataKey );
#endif
		Release( m_pData );
		return;
	}
	
	if ( !m_pData || !m_nBytes ) {
		// allocate
		AllocTLS();
	}
	
	if ( pData ) {
		m_pData = pData;
	} else {
		m_pData = Allocate( nBytes );
	}
	m_nBytes = nBytes;
#ifdef _WIN32
	TlsSetValue( m_nIndex, pData );
#else
	pthread_setspecific( m_hDataKey, pData );
#endif
}

GDR_INLINE uint64_t CThreadLocalData::Flush( void )
{
	uint64_t amount;
	
	if ( !m_pData || !m_nBytes ) {
		return 0;
	}
	
	amount = m_nBytes;
	Release( m_pData );
	
	return amount;
}

GDR_INLINE CThreadMutex::CThreadMutex( void )
{
#ifdef _WIN32
	InitializeSRWLock( &m_hLock );
	m_hOwnerThread = NULL;
	m_nLockCount = 0;
#else
	pthread_mutexattr_init( &m_hAttrib );
	pthread_mutexattr_settype( &m_hAttrib, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &m_hMutex, &m_hAttrib );
#endif
}

GDR_INLINE CThreadMutex::~CThreadMutex()
{
#ifdef _WIN32
#else
	pthread_mutexattr_destroy( &m_hAttrib );
	pthread_mutex_destroy( &m_hMutex );
#endif
}

GDR_INLINE bool CThreadMutex::AssertOwnedByCurrentThread( void ) const
{
#ifdef _WIN32
	return m_hOwnerThread == GetCurrentThreadId();
#else
	return true;
#endif
}

GDR_INLINE void CThreadMutex::Lock( void )
{
#ifdef _WIN32
	const DWORD currentThreadId = GetCurrentThreadId();
	
	if ( m_hOwnerThread == currentThreadId ) {
		// already locked on current thread
		m_nLockCount++;
	} else {
		AcquireSRWLockExclusive( &m_hLock );
		m_hOwnerThread = currentThreadId;
		m_nLockCount = 1;
	}
#else
	pthread_mutex_lock( &m_hMutex );
#endif
}

GDR_INLINE void CThreadMutex::Unlock( void )
{
#ifdef _WIN32
	if ( m_hOwnerThread == GetCurrentThreadId() ) {
		m_nLockCount--;
		
		if ( m_nLockCount == 0 ) {
			m_hOwnerThread = 0;
			ReleaseSRWLockExclusive( &m_hLock );
		}
	}
#else
	pthread_mutex_unlock( &m_hMutex );
#endif
}

template<typename T>
GDR_INLINE CThreadAtomic<T>::CThreadAtomic( void )
	: m_hValue( 0 )
{
}

template<typename T>
GDR_INLINE CThreadAtomic<T>::~CThreadAtomic()
{
}

template<typename T>
GDR_INLINE const CThreadAtomic<T>& CThreadAtomic<T>::operator++( void )
{
	fetch_add();
}

template<typename T>
GDR_INLINE const CThreadAtomic<T>& CThreadAtomic<T>::operator++( int )
{
	fetch_add();
}

template<typename T>
GDR_INLINE const CThreadAtomic<T>& CThreadAtomic<T>::operator--( void )
{
	fetch_sub();
}

template<typename T>
GDR_INLINE const CThreadAtomic<T>& CThreadAtomic<T>::operator--( int )
{
	fetch_sub();
}

template<typename T>
GDR_INLINE const CThreadAtomic<T>& CThreadAtomic<T>::operator+=( const T& value )
{
	add( value );
}

template<typename T>
GDR_INLINE const CThreadAtomic<T>& CThreadAtomic<T>::operator-=( const T& value )
{
	sub( value );
}

template<typename T>
GDR_INLINE CThreadAtomic<T> CThreadAtomic<T>::operator+( const T& rhs ) const
{
	add( rhs );
}

template<typename T>
GDR_INLINE CThreadAtomic<T> CThreadAtomic<T>::operator-( const T& rhs ) const
{
	sub( rhs );
}

template<typename T>
GDR_INLINE const CThreadAtomic<T>& CThreadAtomic<T>::operator=( const T& value )
{
	store( value );
}

template<typename T>
GDR_INLINE const T& CThreadAtomic<T>::operator()( void ) const
{
	return m_hValue;
}

template<typename T>
GDR_INLINE CThreadAtomic<T>::operator T( void ) const
{
	return m_hValue;
}

template<typename T>
GDR_INLINE bool CThreadAtomic<T>::operator!( void ) const
{
	return !(bool)load();
}

template<typename T>
GDR_INLINE bool CThreadAtomic<T>::operator!=( const T& value ) const
{
	return load() != value;
}

template<typename T>
GDR_INLINE bool CThreadAtomic<T>::operator==( const T& value ) const
{
	return load() == value;
}

template<typename T>
GDR_INLINE const T& CThreadAtomic<T>::load( MemoryOrder order ) const
{
#ifdef _WIN32
	return (const T &)InterlockedCompareExchange( (volatile LONG *)&m_hValue, 0, 0 );
#else
	T out;
	__atomic_load( &m_hValue, &out, order );
	return out;
#endif
}

template<typename T>
GDR_INLINE void CThreadAtomic<T>::store( T value, MemoryOrder order )
{
#ifdef _WIN32
	InterlockedExchange( (volatile LONG *)&m_hValue, *(LONG *)&value );
#else
	__atomic_store( &m_hValue, &value, order );
#endif
}

template<typename T>
GDR_INLINE void CThreadAtomic<T>::add( T value, MemoryOrder order )
{
#ifdef _WIN32
	InterlockedExchangeAdd( (volatile LONG *)&m_hValue, *(LONG *)&value );
#else
	__atomic_add_fetch( &value, &m_hValue, order );
#endif
}

template<typename T>
GDR_INLINE void CThreadAtomic<T>::sub( T value, MemoryOrder order )
{
#ifdef _WIN32
	InterlockedExchangeAdd( (volatile LONG *)&m_hValue, -( *(LONG *)&value ) );
#else
	__atomic_sub_fetch( &value, &m_hValue, order );
#endif
}

template<typename T>
GDR_INLINE void CThreadAtomic<T>::fetch_add( MemoryOrder order )
{
	add( 1, order );
}

template<typename T>
GDR_INLINE void CThreadAtomic<T>::fetch_sub( MemoryOrder order )
{
	sub( 1, order );
}

GDR_INLINE CThreadRWMutex::CThreadRWMutex( void )
{
	m_nReaders = 0;
	m_nActiveWriters = 0;
	m_nWaitingWriters = 0;
	
#ifdef _WIN32
	InitializeSRWLock( &m_hLock );
	InitializeConditionVariable( &m_ReadersDoneCond );
#else
	pthread_mutex_init( &m_hMutex, NULL );
	pthread_rwlock_init( &m_hRWLock, NULL );
	pthread_cond_init( &m_hCondition, NULL );
#endif
}

GDR_INLINE CThreadRWMutex::~CThreadRWMutex()
{
#ifdef _WIN32
#else
	pthread_mutex_destroy( &m_hMutex );
	pthread_rwlock_destroy( &m_hRWLock );
	pthread_cond_destroy( &m_hCondition );
#endif
}

GDR_INLINE void CThreadRWMutex::ReadLock( void )
{
#ifdef _WIN32
	AcquireSRWLockShared( &m_hLock );
	
	while ( m_nActiveWriters || m_nWaitingWriters ) {
		SleepConditionVariableSRW( &m_hCondition, &m_hLock, INFINITE, CONDITION_VARIABLE_LOCKMODE_SHARED );
	}
	
	m_nReaders++;
	
	ReleaseSRWLockShared( &m_hLock );
#else
	pthread_rwlock_rdlock( &m_hRWLock );
	
	pthread_mutex_lock( &m_hMutex );
	while ( m_nActiveWriters || m_nWaitingWriters ) {
		pthread_cond_wait( &m_hCondition, &m_hMutex );
	}
	
	m_nReaders++;
	pthread_mutex_unlock( &m_hMutex );
	
	pthread_rwlock_unlock( &m_hRWLock );
#endif
}

GDR_INLINE void CThreadRWMutex::ReadUnlock( void )
{
#ifdef _WIN32
	AcquireSRWLockShared( &m_hLock );
	
	m_nReaders--;
	
	if ( m_nReaders && m_nWaitingWriters ) {
		WakeAllConditionVariable( &m_hCondition );
	}
	
	ReleaseSRWLockShared( &m_hLock );
#else
	pthread_rwlock_rdlock( &m_hRWLock );
	
	pthread_mutex_lock( &m_hMutex );
	m_nReaders--;
	
	if ( m_nReaders && m_nWaitingWriters ) {
		pthread_cond_broadcast( &m_hCondition );
	}
	pthread_mutex_unlock( &m_hMutex );
	
	pthread_rwlock_unlock( &m_hRWLock );
#endif
}

GDR_INLINE void CThreadRWMutex::WriteLock( void )
{
#ifdef _WIN32
	AcquireSRWLockExclusive( &m_hLock );
	
	m_nWaitingWriters++;
	
	while ( m_nReaders || m_nActiveWriters ) {
		SleepConditionVariableSRW( &m_hCondition, &m_hLock, INFINITE, 0 );
	}
	
	m_nWaitingWriters--;
	m_nActiveWriters++;
	
	ReleaseSRWLockExclusive( &m_hLock );
#else
	pthread_rwlock_wrlock( &m_hRWLock );
	
	pthread_mutex_lock( &m_hMutex );
	m_nWaitingWriters++;
	
	while ( m_nReaders || m_nActiveWriters ) {
		pthread_cond_wait( &m_hCondition, &m_hMutex );
	}
	
	m_nWaitingWriters--;
	m_nActiveWriters++;
	pthread_mutex_unlock( &m_hMutex );
	
	pthread_rwlock_unlock( &m_hRWLock );
#endif
}

GDR_INLINE void CThreadRWMutex::WriteUnlock( void )
{
#ifdef _WIN32
	AcquireSRWLockExclusive( &m_hLock );
	
	m_nActiveWriters--;
	
	if ( m_nWaitingWriters ) {
		WakeConditionVariable( &m_hCondition );
	} else {
		WakeAllConditionVariable( &m_hCondition );
	}
	
	ReleaseSRWLockExclusive( &m_hLock );
#else
	pthread_rwlock_wrlock( &m_hRWLock );
	
	pthread_mutex_lock( &m_hMutex );
	m_nActiveWriters--;
	
	if ( m_nWaitingWriters ) {
		pthread_cond_signal( &m_hCondition );
	} else {
		pthread_cond_broadcast( &m_hCondition );
	}
	pthread_mutex_unlock( &m_hMutex );
	
	pthread_rwlock_unlock( &m_hRWLock );
#endif
}

template<typename T>
GDR_INLINE T ReadVolatileMemory( T const *ptr )
{
	volatile const T *pVolatilePtr = (volatile const T*)ptr;
	return *pVolatilePtr;
}


#endif