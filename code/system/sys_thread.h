#ifndef __SYS_THREAD__
#define __SYS_THREAD__

#pragma once

// NOTE: unless you want a stream of useless warnings, don't enable -Wpedantic for any module that includes EASTL/atomic.h or anything like that
#include <EASTL/atomic.h>

// using boost threads until I implement custom, much more engine-focused and personalized threading stuff
typedef boost::recursive_mutex CThreadRecursiveLock;

class CThreadMutex
{
public:
	CThreadMutex( void );
	~CThreadMutex();

	void Lock( void );
	void Lock( void ) const { const_cast<CThreadMutex *>(this)->Lock(); }
	void Unlock( void );
	void Unlock( void ) const { const_cast<CThreadMutex *>(this)->Unlock(); }
	bool TryLock( void );
	bool TryLock( void ) const { const_cast<CThreadMutex *>(this)->TryLock(); }
private:
#if 0

#ifdef _WIN32
	SRWLOCK m_hLock;
#elif defined(POSIX)
	pthread_mutex_t m_hLock;
	pthread_mutexattr_t m_hAttrib;
#endif

#endif
	boost::recursive_mutex m_hLock;
};

GDR_INLINE CThreadMutex::CThreadMutex( void ) {
}

GDR_INLINE CThreadMutex::~CThreadMutex() {
}

GDR_INLINE void CThreadMutex::Lock( void ) {
#if 0

#ifdef _WIN32
	AcquireSRWExclusive( &m_hLock );
#elif defined(POSIX)
	pthread_mutex_lock( &m_hLock );
#endif

#endif
	m_hLock.lock();
}

GDR_INLINE void CThreadMutex::Unlock( void ) {
	m_hLock.unlock();
}

GDR_INLINE bool CThreadMutex::TryLock( void ) {
	return m_hLock.try_lock();
}

typedef void (*threadfunc_t)( void );

class CThread
{
public:
	CThread( void ) { }
	~CThread() { }

	void Join( void );
	bool Joinable( void ) const;

	bool Run( threadfunc_t fn );
private:
	boost::thread m_hThread;
};

GDR_INLINE void CThread::Join( void ) {
	m_hThread.join();
}

GDR_INLINE bool CThread::Joinable( void ) const {
	return m_hThread.joinable();
}

GDR_INLINE bool CThread::Run( threadfunc_t fn ) {
	m_hThread = boost::thread( [&]( void ) -> void { fn(); } );

	return true;
}

class CThreadSpinRWLock
{
public:
	CThreadSpinRWLock( void );
	~CThreadSpinRWLock();

	void Lock( void );
	void Unlock( void );
	bool TryLock( void );
private:
	eastl::atomic_flag m_hFlag;
};

GDR_INLINE CThreadSpinRWLock::CThreadSpinRWLock( void ) {
}

GDR_INLINE CThreadSpinRWLock::~CThreadSpinRWLock() {
}

GDR_INLINE void CThreadSpinRWLock::Lock( void ) {
	while (m_hFlag.test_and_set( eastl::memory_order_acquire ));
}

GDR_INLINE void CThreadSpinRWLock::Unlock( void ) {
	m_hFlag.clear( eastl::memory_order_release );
}

GDR_INLINE bool CThreadSpinRWLock::TryLock( void ) {
	return !m_hFlag.test_and_set( eastl::memory_order_acquire );
}

//
// CThreadAutoLock: used for scope based mutex locks
//
template<typename Mutex>
class CThreadAutoLock
{
public:
	GDR_INLINE CThreadAutoLock( Mutex& lock )
		: m_pLock( lock )
	{
		m_pLock.Lock();
	}
	GDR_INLINE ~CThreadAutoLock()
	{
		m_pLock.Unlock();
	}

	GDR_INLINE void Lock( void ) { m_pLock.Lock(); }
	GDR_INLINE void Unlock( void ) { m_pLock.Unlock(); }
	GDR_INLINE bool TryLock( void ) { return m_pLock.TryLock(); }
private:
	Mutex& m_pLock;
};

#if 0
#ifdef _WIN32
typedef HANDLE sys_thread_t;
typedef SRWLOCK sys_mutex_t;
typedef CRITICAL_SECTION sys_recursive_mutex_t;
typedef DWORD sys_thread_id_t;
#else
typedef pthread_t sys_thread_t;
typedef pthread_mutex_t sys_mutex_t;
typedef pthread_mutex_t sys_recursive_mutex_t;
typedef pthread_t sys_thread_id_t;
#endif

#if defined(_NOMAD_DEBUG)
typedef struct ownerThread_s
{
	eastl::atomic<sys_thread_id_t> m_OwnerThread;
	constexpr ownerThread_s( void ) {
		m_OwnerThread = 0;
	}

	static void on_deadlock( void ) {
		N_Error( ERR_FATAL, "[Thread Debug]: recursive locking on a non-rescursive mutex!" );
	}
	sys_thread_id_t checkOwnerBeforeLock( void ) const {
		sys_thread_id_t self;
	#ifdef _WIN32
		self = GetCurrentThreadId();
	#else
		self = pthread_self();
	#endif
		if (m_OwnerThread.load( eastl::memory_order_relaxed ) == self) {
			on_deadlock();
		}
		return self;
	}
	void setOwnerAfterLock( sys_thread_id_t id ) {
		m_OwnerThread.store( id, eastl::memory_order_relaxed );
	}
	void checkSetOwnerBeforeUnlock( void ) {
		sys_thread_id_t self;
	#ifdef _WIN32
		self = GetCurrentThreadId();
	#else
		self = pthread_self();
	#endif
		if (m_OwnerThread.load( eastl::memory_order_relaxed ) != self) {
			on_deadlock();
		}
		m_OwnerThread.store( 0, eastl::memory_order_relaxed );
	}
} ownerThread_t;
#endif

#if 0
class CThreadSharedRWLock
{
public:
	CThreadSharedRWLock( void );
	~CThreadSharedRWLock();

	void *GetNative( void );
	const void *GetNative( void ) const;

	void Lock( void );
	void Lock( void ) const;
	void Unlock( void );
	void Unlock( void ) const;
	bool TryLock( void );
	bool TryLock( void ) const;
private:
#ifdef POSIX
	sys_mutex_t m_hLock;
	pthread_mutexattr_t m_hLockAttrib;
#endif
	eastl::atomic<uint_fast16_t> m_nCounter;
#ifdef _NOMAD_DEBUG
	ownerThread_t m_hOwnerThread;
#endif
};

GDR_INLINE CThreadSharedRWLock::CThreadSharedRWLock( void )
{
#ifdef POSIX
	pthread_mutex_init( &m_hLock );
	pthread_mutexattr_init( &m_hLockAttrib );
#endif
}

GDR_INLINE CThreadSharedRWLock::~CThreadSharedRWLock() {
	// terminate if someone tries to destroy an owned mutex
	Assert( m_nCounter.load( eastl::memory_order_relaxed ) == 0 );
}
#endif


class CThreadRecursiveMutex
{
public:
	CThreadRecursiveMutex( void );
	~CThreadRecursiveMutex();

	void *GetNative( void );
	const void *GetNative( void ) const;

	void Lock( void );
	void Lock( void ) const;
	void Unlock( void );
	void Unlock( void ) const;
	bool TryLock( void );
	bool TryLock( void ) const;
private:
	sys_recursive_mutex_t m_hLock;
#ifdef POSIX
	pthread_mutexattr_t m_hLockAttrib;
#endif
};

GDR_INLINE CThreadRecursiveMutex::CThreadRecursiveMutex( void )
{
#ifdef _WIN32
	InitializeCriticalSection( &m_hLock );
#else
	pthread_mutexattr_init( &m_hLockAttrib );
	pthread_mutexattr_settype( &m_hLockAttrib, PTHREAD_MUTEX_RECURSIVE );

	pthread_mutex_init( &m_hLock, &m_hLockAttrib );
#endif
}

GDR_INLINE CThreadRecursiveMutex::~CThreadRecursiveMutex( void )
{
#ifdef _WIN32
	DeleteCriticalSection( &m_hLock );
#else
	pthread_mutex_destroy( &m_hLock );
	pthread_mutexattr_destroy( &m_hLockAttrib );
#endif
}

GDR_INLINE void *CThreadRecursiveMutex::GetNative( void ) {
	return &m_hLock;
}

GDR_INLINE const void *CThreadRecursiveMutex::GetNative( void ) const {
	return &m_hLock;
}

GDR_INLINE void CThreadRecursiveMutex::Lock( void )
{
#ifdef _WIN32
	EnterCriticalSection( &m_hLock );
#else
	pthread_mutex_lock( &m_hLock );
#endif
}

GDR_INLINE void CThreadRecursiveMutex::Lock( void ) const
{
#ifdef _WIN32
	EnterCriticalSection( &const_cast<CThreadRecursiveMutex *>(this)->m_hLock );
#else
	pthread_mutex_lock( &const_cast<CThreadRecursiveMutex *>(this)->m_hLock );
#endif
}

GDR_INLINE void CThreadRecursiveMutex::Unlock( void )
{
#ifdef _WIN32
	LeaveCriticalSection( &m_hLock );
#else
	pthread_mutex_unlock( &m_hLock );
#endif
}

GDR_INLINE void CThreadRecursiveMutex::Unlock( void ) const
{
#ifdef _WIN32
	LeaveCriticalSection( &const_cast<CThreadRecursiveMutex *>(this)->m_hLock );
#else
	pthread_mutex_unlock( &const_cast<CThreadRecursiveMutex *>(this)->m_hLock );
#endif
}

GDR_INLINE bool CThreadRecursiveMutex::TryLock( void )
{
#ifdef _WIN32
	return (TryCriticalSection( &m_hLock ) != 0);
#else
	return pthread_mutex_trylock( &m_hLock ) == 0;
#endif
}

GDR_INLINE void CThreadRecursiveMutex::TryLock( void ) const
{
#ifdef _WIN32
	return (TryCriticalSection( &const_cast<CThreadRecursiveMutex *>(this)->m_hLock ) != 0);
#else
	return pthread_mutex_trylock( &const_cast<CThreadRecursiveMutex *>(this)->m_hLock ) == 0;
#endif
}

class CThreadMutex
{
public:
	CThreadMutex( void );
	~CThreadMutex();

	void *GetNative( void );
	const void *GetNative( void ) const;

	void Lock( void );
	void Lock( void ) const;
	void Unlock( void );
	void Unlock( void ) const;
	bool TryLock( void );
	bool TryLock( void ) const;
private:
	sys_mutex_t m_hLock;
#ifdef _NOMAD_DEBUG
	ownerThread_t m_hOwnerThread;
#endif
#ifdef _WIN32
#elif defined(POSIX)
	pthread_mutexattr_t m_hLockAttrib;
#endif
};

GDR_INLINE CThreadMutex::CThreadMutex( void )
{
#ifdef POSIX
	pthread_mutexattr_init( &m_hLockAttrib );
	pthread_mutexattr_settype( &m_hLockAttrib, PTHREAD_MUTEX_NORMAL );

	pthread_mutex_init( &m_hLock, &m_hLockAttrib );
#endif
}

GDR_INLINE CThreadMutex::~CThreadMutex()
{
#ifdef POSIX
	pthread_mutexattr_destroy( &m_hLockAttrib );
	pthread_mutex_destroy( &m_hLock );
#endif
}

GDR_INLINE void *CThreadMutex::GetNative( void ) {
	return &m_hLock;
}

GDR_INLINE const void *CThreadMutex::GetNative( void ) const {
	return &m_hLock;
}

GDR_INLINE void CThreadMutex::Lock( void )
{
#ifdef _NOMAD_DEBUG
	const sys_thread_id_t self = m_hOwnerThread.checkOwnerBeforeLock();
#endif
#ifdef _WIN32
	AcquireSRWLockExclusive( &m_hLock );
#elif defined(POSIX)
	pthread_mutex_lock( &m_hLock );
#endif
#ifdef _NOMAD_DEBUG
	m_hOwnerThread.setOwnerAfterLock( self );
#endif
}

GDR_INLINE void CThreadMutex::Lock( void ) const
{
#ifdef _NOMAD_DEBUG
	const sys_thread_id_t self = m_hOwnerThread.checkOwnerBeforeLock();
#endif
#ifdef _WIN32
	AcquireSRWLockExclusive( &const_cast<CThreadMutex *>(this)->m_hLock );
#elif defined(POSIX)
	pthread_mutex_lock( &const_cast<CThreadMutex *>(this)->m_hLock );
#endif
#ifdef _NOMAD_DEBUG
	const_cast<CThreadMutex *>(this)->m_hOwnerThread.setOwnerAfterLock( self );
#endif
}

GDR_INLINE void CThreadMutex::Unlock( void )
{
#ifdef _NOMAD_DEBUG
	m_hOwnerThread.checkSetOwnerBeforeUnlock();
#endif
#ifdef _WIN32
	ReleaseSRWLockExclusive( &m_hLock );
#elif defined(POSIX)
	pthread_mutex_unlock( &m_hLock );
#endif
}

GDR_INLINE void CThreadMutex::Unlock( void ) const
{
#ifdef _NOMAD_DEBUG
	const_cast<CThreadMutex *>(this)->m_hOwnerThread.checkSetOwnerBeforeUnlock();
#endif
#ifdef _WIN32
	ReleaseSRWLockExclusive( &const_cast<CThreadMutex *>(this)->m_hLock );
#elif defined(POSIX)
	pthread_mutex_unlock( &const_cast<CThreadMutex *>(this)->m_hLock );
#endif
}

GDR_INLINE bool CThreadMutex::TryLock( void )
{
#ifdef _NOMAD_DEBUG
	const sys_thread_id_t self = m_hOwnerThread.checkOwnerBeforeLock();
#endif
#ifdef _WIN32
	const BOOL ret = TryAcquireSRWLockExclusive( &m_hLock );
#else
	const int ret = pthread_mutex_trylock( &m_hLock );
#endif
#ifdef _NOMAD_DEBUG
	if (ret) {
		m_hOwnerThread.setOwnerAfterLock( self );
	}
#endif
	return (bool)ret;
}

typedef struct {
	uint32_t numArgs;
	void *threadArgs;
} threadResult_t;

typedef void (*threadfunc_t)( void *args );

class CThread
{
public:
	CThread( void );
	~CThread();

	void SetName( const char *name );
	void Join( void );
	bool Joinable( void ) const;
	
	const char *GetName( void ) const;

	bool Run( threadfunc_t fn, uint32_t numArgs, void *threadArgs );
private:
	char name[MAX_GDR_PATH];

	sys_thread_t m_hThread;
	sys_thread_id_t m_iThreadId;
};

GDR_INLINE CThread::CThread( void )
{
	memset( name, 0, sizeof(name) );

	m_iThreadId = 0;
	m_hThread = (sys_thread_t)0;
}

GDR_INLINE CThread::~CThread()
{

}
#endif

#endif