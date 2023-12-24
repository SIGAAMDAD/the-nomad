#ifndef __THREAD_INL__
#define __THREAD_INL__

GDR_INLINE CThread::CThread( void )
{
#ifdef _WIN32
	m_hThread = NULL;
#else
	m_ThreadId = 0;
	pthread_attr_init( &m_hAttrib );
#endif
	m_szName[0] = 0;
	m_iResult = 0;
}

GDR_INLINE CThread::~CThread()
{
#ifdef _WIN32
	if (m_hThread) 
#elif defined(POSIX)
	if (m_ThreadId)
#endif
	{
		if (IsAlive()) {
			const char *msg =
			"Illegal termination of worker thread! Threads must negotiate an end to the thread before the CThread object is destroyed.\n";
			
			Con_Printf( "%s", msg );
		#ifdef _WIN32
			Sys_DebugMessageBox( "Assertion Failure", va("Line: %u\nFile: %s\nMessage: %s", __LINE__, __FILE__, msg) );
		#endif
		}
	}
}

//---------------------------------------------------------
//
// Return true if the thread has been created and hasn't yet exited
//

GDR_INLINE bool CThread::IsAlive( void ) const {
#ifdef _WIN32
	DWORD dwExitCode;
	return (
		m_hThread 
		&& GetExitCodeThread(m_hThread, &dwExitCode) 
		&& dwExitCode == STILL_ACTIVE );
#elif defined(POSIX)
	return !!m_ThreadId;
#endif
}

GDR_INLINE void CThread::SetName( const char *name ) {
	CThreadAutoLock<CThreadMutex> lock( m_hLock );
	strcpy( m_szName, name );
}

GDR_INLINE const char *CThread::GetName( void ) const {
	return m_szName;
}

#define STACK_SIZE_DEFAULT (1024*2048)

typedef struct {
	CThread *pThread;
	bool bInitSuccess;
} threadrun_t;

#ifdef _WIN32
inline uint64_t __stdcall ThreadProc( void *arg )
#else
inline void *ThreadProc( void *arg )
#endif
{
	CThread *thread;

	thread = (CThread *)arg;

	try {
		thread->m_bInitSuccess = thread->Init();
	} catch (...) {
	}
	
	if (!thread->m_bInitSuccess)
		return 0;
	
#ifdef ALLOW_THREAD_EXCEPTIONS
	if (!Sys_IsInDebugSession()) {
		try {
			thread->m_iResult = thread->Run();
		} catch (...) {
		}
	}
	else
#endif
	{
		thread->m_iResult = thread->Run();
	}
	
	thread->OnExit();
	
	CThreadAutoLock<CThreadMutex> lock( thread->m_hLock );
#ifdef _WIN32
	CloseHandle( thread->m_hThread );
	
	return thread->m_iResult;
#else
	return (void *)(intptr_t)thread->m_iResult;
#endif
}

GDR_INLINE void CThread::OnExit( void ) {
}

GDR_INLINE CThread::ThreadProc_t CThread::GetThreadProc( void ) {
	return ThreadProc;
}

GDR_INLINE bool CThread::Join( uint64_t nTimeout )
{
	Sys_DebugString( va("Joining thread '%s'...\n", m_szName) );
#ifdef _WIN32
	DWORD dwWait = WaitForSingleObject( (HANDLE)m_hThread, nTimeout );
	if ( dwWait == WAIT_TIMEOUT)
		return false;
	if ( dwWait != WAIT_OBJECT_0 && ( dwWait != WAIT_FAILED && GetLastError() != 0 ) ) {
		Assert( 0 );
		return false;
	}
#elif defined(POSIX)
    if (nTimeout != 0) {
        struct timespec abstime;
        
        abstime.tv_sec = nTimeout;
        if (pthread_timedjoin_np( m_ThreadId, NULL, &abstime ) != 0) {
            if (errno != ETIMEDOUT) {
                N_Error( ERR_FATAL, "Error when joining thread" );
            }
        }
    }
    else {
        if (pthread_join( m_ThreadId, NULL ) != 0) {
            N_Error( ERR_FATAL, "Error when joining thread, %s", strerror(errno));
        }
    }
    m_ThreadId = 0;
#endif
	
    Sys_DebugString( "Joined.\n" );
    return true;
}

GDR_INLINE bool CThread::Start( uint64_t nBytesStack )
{	
	if (IsAlive()) {
		AssertMsg( false, "Tried to create a thread that has already been created!" );
		return false;
	}
	
	if (!nBytesStack) {
		nBytesStack = STACK_SIZE_DEFAULT;
	}
	
#ifdef _WIN32
	m_hThread = (HANDLE)CreateThread(
		NULL,
		nBytesStack,
		(LPTHREAD_START_ROUTINE)GetThreadProc(),
		this,
		0,
		NULL);
	
	if (!m_hThread) {
		N_Error( ERR_DROP, "CThread::Run: CreateThread failed, error: 0x%04x", GetLastError() );
	}
#elif defined(POSIX)
	pthread_attr_setstacksize( &m_hAttrib, nBytesStack );
	if (pthread_create( &m_ThreadId, &m_hAttrib, (void *(*)(void *))GetThreadProc(), this ) != 0) {
		N_Error( ERR_DROP, "CThread::Run: pthread_create failed, error: %s", strerror(errno) );
	}
#endif
    
    return true;
}

// force hard termination of a thread. Used for fatal errors
GDR_INLINE bool CThread::Terminate( int iExitCode )
{
#ifdef _WIN32
	// I dearly hope you know what you're doing!
	if (!TerminateThread( m_hThread, iExitCode ))
		return false;
	
	CloseHandle( m_hThread );
	m_hThread = NULL;
#elif defined(POSIX)
	pthread_kill( m_ThreadId, SIGKILL );
	m_ThreadId = 0;
#endif
	return true;
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
	::Sleep(0);
#elif defined( _PS3 )
	// sys_ppu_thread_yield doesn't seem to function properly, so sleep instead.
	sys_timer_usleep( 60 );
#elif defined(POSIX)
	sched_yield();
#endif
}

//---------------------------------------------------------
//
// This method causes the current thread to yield and not to be
// scheduled for further execution until a certain amount of real
// time has elapsed, more or less. Duration is in milliseconds

GDR_INLINE void CThread::Sleep( uint64_t nDuration )
{
#ifdef _WIN32
	::Sleep(nDuration);
#elif defined(POSIX)
	usleep( nDuration * 1000 );
#endif
}

GDR_INLINE void CThread::Stop( int iExitCode )
{
#ifdef _WIN32
	
#elif defined(POSIX)
	pthread_kill( m_ThreadId, SIGSTOP );
#endif
}

#endif