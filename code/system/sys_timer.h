#ifndef __SYS_TIMER__
#define __SYS_TIMER__

#pragma once

#include <EASTL/chrono.h>

class CTimer
{
public:
    CTimer( void )
        : m_nStartTime( Sys_Milliseconds() ), m_nEndTime( 0 ), m_bRunning( qtrue )
    {
    }
    CTimer( int64_t milliseconds, int64_t seconds, int32_t minutes )
        : m_nStartTime( milliseconds ), m_nEndTime( 0 )
    {
    }
    ~CTimer() {
        Stop();
    }
    const CTimer& operator=( const CTimer& other ) {
        m_nStartTime = other.m_nStartTime;
        m_nEndTime = other.m_nEndTime;

        return *this;
    }

    void Run( void ) {
        m_bRunning = qtrue;
        m_nStartTime = Sys_Milliseconds();
    }
    void Stop( void ) {
        if ( !m_bRunning ) {
            return;
        }
        m_bRunning = qfalse;
        m_nEndTime = Sys_Milliseconds();
    }

    int64_t ElapsedMilliseconds( void ) const {
        return ( m_nEndTime - m_nStartTime );
    }

    int64_t ElapsedSeconds( void ) const {
        return ( m_nEndTime - m_nStartTime ) / 1000;
    }
    
    int32_t ElapsedMinutes( void ) const {
        return ( m_nEndTime - m_nStartTime ) / 60000;
    }
private:
    uint64_t m_nStartTime;
    uint64_t m_nEndTime;
    qboolean m_bRunning;
};

#endif