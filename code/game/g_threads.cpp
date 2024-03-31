#include "g_game.h"
#include "g_threads.h"
#include <jemalloc/jemalloc.h>

CRenderThread *g_pRenderThread;

CRenderThread::CRenderThread( void ) {
}

CRenderThread::~CRenderThread() {
}

#include <easy/profiler.h>
int CRenderThread::Run( void ) {
#ifdef _NOMAD_DEBUG
    EASY_NONSCOPED_BLOCK( "Render Thread" );
#endif

    if ( com_errorEntered ) {
        return -1;
    }

    re.BeginFrame( STEREO_CENTER );
    
    return 1;
}

bool CRenderThread::Init( void ) {
    return true;
}

void CRenderThread::OnExit( void ) {
    re.EndFrame( NULL, NULL );

    PROFILE_BLOCK_END;
}
