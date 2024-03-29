#ifndef __G_THREADS__
#define __G_THREADS__

#pragma once

#include "../engine/n_threads.h"

class CRenderThread : public CThread
{
public:
    CRenderThread( void  );
    virtual ~CRenderThread();
private:
    virtual bool Init( void ) override;
    virtual int Run( void ) override;
    virtual void OnExit( void ) override;

    qboolean m_bRendering;
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
};

extern CRenderThread *g_pRenderThread;

#endif
