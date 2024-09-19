#include "g_game.h"
#include <EASTL/atomic.h>

extern cvar_t *sys_forceSingleThreading;

#ifdef _WIN32

#else
#include <pthread.h>

static pthread_mutex_t smpMutex = PTHREAD_MUTEX_INITIALIZER;

//static eastl::atomic<qboolean> renderCommandsEvent( qfalse );
//static eastl::atomic<qboolean> renderCompletedEvent( qfalse );
static pthread_cond_t renderCommandsEvent = PTHREAD_COND_INITIALIZER;
static pthread_cond_t renderCompletedEvent = PTHREAD_COND_INITIALIZER;

static void (*glimpRenderThread)( void );

extern SDL_Window *SDL_window;
extern SDL_GLContext SDL_glContext;

static void *GLimp_RenderThreadWrapper( void *arg )
{
	Con_Printf( "Render thread starting\n" );
	
	glimpRenderThread();

	SDL_GL_MakeCurrent( SDL_window, NULL );

	Con_Printf( "Render thread terminating\n" );

	return arg;
}

qboolean GLimp_SpawnRenderThread( void (*function)( void ) )
{
	pthread_t renderThread;
	int ret;

	pthread_mutex_init( &smpMutex, NULL );

	pthread_cond_init( &renderCommandsEvent, NULL );
	pthread_cond_init( &renderCompletedEvent, NULL );

	glimpRenderThread = function;

	ret = pthread_create( &renderThread, NULL, GLimp_RenderThreadWrapper, NULL );
	if ( ret ) {
		Con_Printf( "pthread_create() returned %i: %s\n", ret, strerror( errno ) );
		return qfalse;
	} else {
		ret = pthread_detach( renderThread );
		if ( ret ) {
			Con_Printf( "pthread_detach() returned %i: %s\n", ret, strerror( errno ) );
		}
	}

	return qtrue;
}

static volatile void *smpData = NULL;
static volatile qboolean smpDataReady;

void *GLimp_RenderSleep( void )
{
	void *data;

	SDL_GL_MakeCurrent( SDL_window, NULL );

	pthread_mutex_lock( &smpMutex );
	{
		smpData = NULL;
		smpDataReady = qfalse;

		// after this, the front end can exit GLimp_FrontEndSleep
		pthread_cond_signal( &renderCompletedEvent );

		while ( !smpDataReady ) {
			pthread_cond_wait( &renderCommandsEvent, &smpMutex );
		}

		data = (void *)smpData;
	}
	pthread_mutex_unlock( &smpMutex );

	SDL_GL_MakeCurrent( SDL_window, SDL_glContext );

	return data;
}

void GLimp_FrontEndSleep( void )
{
	pthread_mutex_lock( &smpMutex );
	{
		while ( smpData ) {
			pthread_cond_wait( &renderCompletedEvent, &smpMutex );
		}
	}
	pthread_mutex_unlock( &smpMutex );

	SDL_GL_MakeCurrent( SDL_window, SDL_glContext );
}

void GLimp_WakeRenderer( void *data )
{
	SDL_GL_MakeCurrent( SDL_window, NULL );

	pthread_mutex_lock( &smpMutex );
	{
		assert( smpData == NULL );
		smpData = data;
		smpDataReady = qtrue;

		// after this, the renderer can continue through GLimp_RenderSleep
		pthread_cond_signal( &renderCommandsEvent );
	}
	pthread_mutex_unlock( &smpMutex );
}
#endif