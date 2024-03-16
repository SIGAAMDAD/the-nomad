#ifndef __SYS_TIMER__
#define __SYS_TIMER__

#pragma once

#include <EASTL/chrono.h>

class CTimer
{
public:
    CTimer( void )
        : m_StartTime( eastl::chrono::system_clock::now() ), m_bRunning( qtrue )
    {
    }
    ~CTimer() {
        Stop();
    }

    void Run( void ) {
        m_bRunning = qtrue;
        m_StartTime = eastl::chrono::system_clock::now();
    }
    void Stop( void ) {
        if (!m_bRunning) {
            return;
        }
        m_bRunning = qfalse;
        m_EndTime = eastl::chrono::system_clock::now();
    }

    eastl::chrono::milliseconds ElapsedMilliseconds( void ) const {
        return eastl::chrono::duration_cast<eastl::chrono::milliseconds>(
            ( m_bRunning ? eastl::chrono::system_clock::now() : m_EndTime ) - m_StartTime
        );
    }

    eastl::chrono::seconds ElapsedSeconds( void ) const {
        return eastl::chrono::duration_cast<eastl::chrono::seconds>(
            ( m_bRunning ? eastl::chrono::system_clock::now() : m_EndTime ) - m_StartTime
        );
    }
    
    eastl::chrono::seconds ElapsedMinutes( void ) const {
        return eastl::chrono::duration_cast<eastl::chrono::minutes>(
            ( m_bRunning ? eastl::chrono::system_clock::now() : m_EndTime ) - m_StartTime
        );
    }

    int64_t ElapsedMilliseconds_ML( void ) const {
        return eastl::chrono::duration_cast<eastl::chrono::milliseconds>(
            ( m_bRunning ? eastl::chrono::system_clock::now() : m_EndTime ) - m_StartTime
        ).count();
    }

    int64_t ElapsedSeconds_ML( void ) const {
        return eastl::chrono::duration_cast<eastl::chrono::seconds>(
            ( m_bRunning ? eastl::chrono::system_clock::now() : m_EndTime ) - m_StartTime
        ).count();
    }
    
    int32_t ElapsedMinutes_ML( void ) const {
        return eastl::chrono::duration_cast<eastl::chrono::minutes>(
            ( m_bRunning ? eastl::chrono::system_clock::now() : m_EndTime ) - m_StartTime
        ).count();
    }
private:
    eastl::chrono::system_clock::time_point m_StartTime;
    eastl::chrono::system_clock::time_point m_EndTime;
    qboolean m_bRunning;
};

#endif