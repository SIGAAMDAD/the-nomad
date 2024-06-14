#ifndef __SYS_TIMER__
#define __SYS_TIMER__

#pragma once

#include <EASTL/chrono.h>

/*
===============================================================================

	Clock tick counter. Should only be used for profiling.

===============================================================================
*/

class CTimer {
public:
	CTimer( void );
	CTimer( uint32_t ms );
	~CTimer( void );

	CTimer operator+( const CTimer& t ) const;
	CTimer operator-( const CTimer& t ) const;
	CTimer&	operator+=( const CTimer& t );
	CTimer&	operator-=( const CTimer& t );
	const CTimer& operator=( const CTimer& t );

	void Start( void );
	void Stop( void );
	void Clear( void );
	uint64_t Milliseconds( void ) const;
    uint64_t Seconds( void ) const;
    uint32_t Minutes( void ) const;
    uint64_t ElapsedMilliseconds( void ) const;
    uint64_t ElapsedSeconds( void ) const;
    uint32_t ElapsedMinutes( void ) const;
private:
	enum {
		TS_STARTED,
		TS_STOPPED
	} state;
	uint64_t start;
	uint64_t ms;
};

/*
=================
CTimer::CTimer
=================
*/
GDR_INLINE CTimer::CTimer( void ) {
	state = TS_STOPPED;
	ms = 0;
}

/*
=================
CTimer::CTimer
=================
*/
GDR_INLINE CTimer::CTimer( uint32_t _ms ) {
	state = TS_STOPPED;
	ms = _ms;
}

/*
=================
CTimer::~CTimer
=================
*/
GDR_INLINE CTimer::~CTimer( void ) {
}

/*
=================
CTimer::operator=
=================
*/
GDR_INLINE const CTimer& CTimer::operator=( const CTimer &t ) {
	start = t.start;
	ms = t.ms;
	state = t.state;
	return *this;
}

/*
=================
CTimer::operator+
=================
*/
GDR_INLINE CTimer CTimer::operator+( const CTimer &t ) const {
	Assert( state == TS_STOPPED && t.state == TS_STOPPED );
	return CTimer( ms + t.ms );
}

/*
=================
CTimer::operator-
=================
*/
GDR_INLINE CTimer CTimer::operator-( const CTimer &t ) const {
	Assert( state == TS_STOPPED && t.state == TS_STOPPED );
	return CTimer( ms - t.ms );
}

/*
=================
CTimer::operator+=
=================
*/
GDR_INLINE CTimer &CTimer::operator+=( const CTimer &t ) {
	Assert( state == TS_STOPPED && t.state == TS_STOPPED );
	ms += t.ms;
	return *this;
}

/*
=================
CTimer::operator-=
=================
*/
GDR_INLINE CTimer &CTimer::operator-=( const CTimer &t ) {
	Assert( state == TS_STOPPED && t.state == TS_STOPPED );
	ms -= t.ms;
	return *this;
}

/*
=================
CTimer::Start
=================
*/
GDR_INLINE void CTimer::Start( void ) {
//	Assert( state == TS_STOPPED );
	state = TS_STARTED;
	start = Sys_Milliseconds();
}

/*
=================
CTimer::Stop
=================
*/
GDR_INLINE void CTimer::Stop( void ) {
//	Assert( state == TS_STARTED );
	ms += Sys_Milliseconds() - start;
	state = TS_STOPPED;
}

/*
=================
CTimer::Clear
=================
*/
GDR_INLINE void CTimer::Clear( void ) {
	ms = 0;
}

/*
=================
CTimer::Milliseconds
=================
*/
GDR_INLINE uint64_t CTimer::Milliseconds( void ) const {
	Assert( state == TS_STOPPED );
	return ms;
}

/*
=================
CTimer::Seconds
=================
*/
GDR_INLINE uint64_t CTimer::Seconds( void ) const {
	Assert( state == TS_STOPPED );
	return ms / 1000;
}

/*
=================
CTimer::Minutes
=================
*/
GDR_INLINE uint32_t CTimer::Minutes( void ) const {
	Assert( state == TS_STOPPED );
	return ms / 60000;
}

/*
=================
CTimer::ElapsedMilliseconds
=================
*/
GDR_INLINE uint64_t CTimer::ElapsedMilliseconds( void ) const {
	return Sys_Milliseconds() - start;
}

/*
=================
CTimer::ElapsedSeconds
=================
*/
GDR_INLINE uint64_t CTimer::ElapsedSeconds( void ) const {
	return ElapsedMilliseconds() / 1000;
}

/*
=================
CTimer::ElapsedMinutes
=================
*/
GDR_INLINE uint32_t CTimer::ElapsedMinutes( void ) const {
	return ElapsedMilliseconds() / 60000;
}

#endif