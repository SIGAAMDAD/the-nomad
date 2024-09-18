#include "g_game.h"
#include <EASTL/atomic.h>

static eastl::atomic<qboolean> s_bExitRenderThread;
static eastl::atomic<qboolean> s_bCanRender;

extern cvar_t *sys_forceSingleThreading;

#ifdef _WIN32

#else
#include <pthread.h>

static pthread_t s_hRenderThread;
static pthread_mutex_t s_hContextLock;

static void *RenderThread( void * )
{
	extern SDL_Window *SDL_window;

	while ( 1 ) {
		if ( s_bExitRenderThread.load() ) {
			break;
		}
		while ( !s_bCanRender.load() )
			;
		
		pthread_mutex_lock( &s_hContextLock );

		pthread_mutex_unlock( &s_hContextLock );
	}

	return NULL;
}

void G_StartupRenderThread( void )
{
	if ( sys_forceSingleThreading->i ) {
		return;
	}


}

void G_ShutdownRenderThread( void )
{
	if ( sys_forceSingleThreading->i ) {
		return;
	}


}
#endif