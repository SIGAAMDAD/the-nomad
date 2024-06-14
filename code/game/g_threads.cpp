#include "g_game.h"
#include "g_threads.h"
#include <jemalloc/jemalloc.h>

CRenderThread *g_pRenderThread;

CRenderThread::CRenderThread( void ) {
}

CRenderThread::~CRenderThread() {
}

bool CRenderThread::Init( void ) {
    m_bRendering.store( false );
    m_bSubmitting.store( false );

    return true;
}

int CRenderThread::Run( void ) {
    if ( com_errorEntered ) {
        return -1;
    }

    m_Timer.Start();

    re.BeginFrame( STEREO_CENTER );

    return 1;
}

void CRenderThread::ClearScene( void ) {
    while ( m_bRendering.load() || m_bSubmitting.load() )
        ;
    
    m_bSubmitting.store( true );
    re.ClearScene();
    m_bSubmitting.store( false );
}

void CRenderThread::RenderScene( const renderSceneRef_t *fd ) {
    while ( m_bRendering.load() || m_bSubmitting.load() )
        ;
    
    m_bSubmitting.store( true );
    re.RenderScene( fd );
    m_bSubmitting.store( false );
}

void CRenderThread::AddSpriteToScene( const vec3_t origin, nhandle_t hSpriteSheet, nhandle_t hSprite, qboolean bNoSpriteSheet ) {
    while ( m_bRendering.load() || m_bSubmitting.load() )
        ;
    
    m_bSubmitting.store( true );
    re.AddSpriteToScene( origin, hSpriteSheet, hSprite, bNoSpriteSheet );
    m_bSubmitting.store( false );
}

void CRenderThread::AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts ) {
    while ( m_bRendering.load() || m_bSubmitting.load() )
        ;
    
    m_bSubmitting.store( true );
    re.AddPolyToScene( hShader, verts, numVerts );
    m_bSubmitting.store( false );
}

void CRenderThread::AddPolyListToScene( const poly_t *polys, uint32_t numPolys ) {
    while ( m_bRendering.load() || m_bSubmitting.load() )
        ;
    
    m_bSubmitting.store( true );
    re.AddPolyListToScene( polys, numPolys );
    m_bSubmitting.store( false );
}

void CRenderThread::AddEntityToScene( const renderEntityRef_t *ent ) {
    while ( m_bRendering.load() || m_bSubmitting.load() )
        ;
    
    m_bSubmitting.store( true );
    re.AddEntityToScene( ent );
    m_bSubmitting.store( false );
}

void CRenderThread::DrawImage( float x, float y, float w, float h, float u1, float v1, float u2, float v2, nhandle_t hShader ) {
    while ( m_bRendering.load() || m_bSubmitting.load() )
        ;
    
    m_bSubmitting.store( true );
    re.DrawImage( x, y, w, h, u1, v1, u2, v2, hShader );
    m_bSubmitting.store( false );
}

void CRenderThread::OnExit( void ) {
    while ( m_bSubmitting.load() )
        ;

    m_bRendering.store( true );
    re.EndFrame( &time_frontend, &time_backend, &gi.pc );
    m_bRendering.store( false );

    m_Timer.Stop();
}
