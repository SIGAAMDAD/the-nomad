#ifndef __G_THREADS__
#define __G_THREADS__

#pragma once

#include "../engine/n_threads.h"
#include <EASTL/internal/atomic/atomic.h>

class CRenderThread : public CThread
{
public:
    CRenderThread( void  );
    virtual ~CRenderThread();

    void ClearScene( void );
    void RenderScene( const renderSceneRef_t *fd );
    void AddSpriteToScene( const vec3_t origin, nhandle_t hSpriteSheet, nhandle_t hSprite, qboolean bNoSpriteSheet );
    void AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );
    void AddPolyListToScene( const poly_t *polys, uint32_t numPolys );
    void AddEntityToScene( const renderEntityRef_t *ent );
    void DrawImage( float x, float y, float w, float h, float u1, float v1, float u2, float v2, nhandle_t hShader );
private:
    virtual bool Init( void ) override;
    virtual int Run( void ) override;
    virtual void OnExit( void ) override;

    eastl::atomic<bool> m_bSubmitting;
    eastl::atomic<bool> m_bRendering;

    CTimer m_Timer;
};

class CSoundThread : public CThread
{
public:
    CSoundThread( void );
    virtual ~CSoundThread();
private:
    virtual bool Init( void ) override;
    virtual int Run( void ) override;
    virtual void OnExit( void ) override;

    CTimer m_Timer;
};

extern CRenderThread *g_pRenderThread;

#endif
