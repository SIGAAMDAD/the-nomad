#ifndef __SYS_TIMER__
#define __SYS_TIMER__

#pragma once

#include <EASTL/chrono.h>

class CTimer
{
public:
    GDR_INLINE CTimer( void )
        : m_StartTime( eastl::chrono::system_clock::now() ), m_bRunning( qtrue )
    {
    }
    GDR_INLINE ~CTimer() {
        Stop();
    }

    GDR_INLINE void Run( void ) {
        m_bRunning = qtrue;
        m_StartTime = eastl::chrono::system_clock::now();
    }
    GDR_INLINE void Stop( void ) {
        if (!m_bRunning) {
            return;
        }
        m_bRunning = qfalse;
        m_EndTime = eastl::chrono::system_clock::now();
    }

    GDR_INLINE eastl::chrono::milliseconds ElapsedMilliseconds( void ) const {
        return eastl::chrono::duration_cast<eastl::chrono::milliseconds>(
            (m_bRunning ? eastl::chrono::system_clock::now() : m_EndTime) - m_StartTime
        );
    }

    GDR_INLINE eastl::chrono::seconds ElapsedSeconds( void ) const {
        return eastl::chrono::duration_cast<eastl::chrono::seconds>(
            (m_bRunning ? eastl::chrono::system_clock::now() : m_EndTime) - m_StartTime
        );
    }
private:
    eastl::chrono::system_clock::time_point m_StartTime;
    eastl::chrono::system_clock::time_point m_EndTime;
    qboolean m_bRunning;
};

#endif