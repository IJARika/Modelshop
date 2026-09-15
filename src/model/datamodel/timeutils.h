//====== Copyright © 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#pragma once

enum RoundStyle_t
{
	ROUND_NEAREST,
	ROUND_DOWN,
	ROUND_UP,
};

class DmeTime_t;

class DmeFramerate_t
{
public:
	/*DmeFramerate_t( float fps );
	DmeFramerate_t( int fps = 0 );*/
	DmeFramerate_t( const DmeFramerate_t& src ) : m_num( src.m_num ), m_den( src.m_den ) {}

	/*void SetFramerate( float flFrameRate );
	void SetFramerate( int fps );*/
	// other (uncommon) options besides 30(29.97 - ntsc video) are 24 (23.976 - ntsc film) and 60 (59.94 - ntsc progressive)
	//void SetFramerateNTSC( int multiplier = 30 );

	/*float GetFramesPerSecond() const;
	DmeTime_t GetTimePerFrame() const;*/

	bool operator==( DmeFramerate_t f ) const { return m_num == f.m_num && m_den == f.m_den; }
	bool operator!=( DmeFramerate_t f ) const { return m_num != f.m_num && m_den != f.m_den; }
	bool operator< ( DmeFramerate_t f ) const { return m_num * ( int )f.m_den <  f.m_num * ( int )m_den; }
	bool operator> ( DmeFramerate_t f ) const { return m_num * ( int )f.m_den >  f.m_num * ( int )m_den; }
	bool operator<=( DmeFramerate_t f ) const { return m_num * ( int )f.m_den <= f.m_num * ( int )m_den; }
	bool operator>=( DmeFramerate_t f ) const { return m_num * ( int )f.m_den >= f.m_num * ( int )m_den; }

	DmeFramerate_t operator*( int i ) const { return DmeFramerate_t( m_num * i, m_den ); }
	DmeFramerate_t operator/( int i ) const { return DmeFramerate_t( m_num, m_den * i ); }

	DmeFramerate_t operator*=( int i ) { assert( abs( m_num * i ) <= USHRT_MAX ); m_num *= ( unsigned short )i; return *this; }
	DmeFramerate_t operator/=( int i ) { assert( abs( m_den * i ) <= USHRT_MAX ); m_den *= ( unsigned short )i; return *this; }

private:
	DmeFramerate_t( int nNumerator, int nDenominator );

	unsigned short m_num;
	unsigned short m_den;

	friend class DmeTime_t;
};

#define DMETIME_ZERO		DmeTime_t(0)
#define DMETIME_MINDELTA	DmeTime_t::MinTimeDelta()
#define DMETIME_MINTIME		DmeTime_t::MinTime()
#define DMETIME_MAXTIME		DmeTime_t::MaxTime()
#define DMETIME_INVALID		DmeTime_t::InvalidTime()

class DmeTime_t
{
public:
	DmeTime_t() : m_tms( INT_MIN ) {} // invalid time
	DmeTime_t(int tms) : m_tms(tms) {}
	DmeTime_t(__int64 tms) : m_tms(static_cast<int>(tms)) {}
	DmeTime_t(float ts) : m_tms(static_cast<int>(ts * 10000.f)) {};


	// time operators

	friend bool operator==( DmeTime_t a, DmeTime_t b ) { return a.m_tms == b.m_tms; }
	friend bool operator!=( DmeTime_t a, DmeTime_t b ) { return a.m_tms != b.m_tms; }
	friend bool operator< ( DmeTime_t a, DmeTime_t b ) { return a.m_tms <  b.m_tms; }
	friend bool operator> ( DmeTime_t a, DmeTime_t b ) { return a.m_tms >  b.m_tms; }
	friend bool operator<=( DmeTime_t a, DmeTime_t b ) { return a.m_tms <= b.m_tms; }
	friend bool operator>=( DmeTime_t a, DmeTime_t b ) { return a.m_tms >= b.m_tms; }

	friend DmeTime_t operator%( DmeTime_t a, DmeTime_t b ) { return DmeTime_t( a.m_tms % b.m_tms ); }
	friend DmeTime_t operator+( DmeTime_t a, DmeTime_t b ) { return DmeTime_t( a.m_tms + b.m_tms ); }
	friend DmeTime_t operator-( DmeTime_t a, DmeTime_t b ) { return DmeTime_t( a.m_tms - b.m_tms ); }

	DmeTime_t operator-() const { return DmeTime_t( -m_tms ); }

	DmeTime_t operator+=( DmeTime_t t ) { m_tms += t.m_tms; return *this; }
	DmeTime_t operator-=( DmeTime_t t ) { m_tms -= t.m_tms; return *this; }

	friend float     operator/( DmeTime_t n, DmeTime_t d ) { return float( n.m_tms / double( d.m_tms ) ); }

	DmeTime_t operator++() { ++m_tms; return *this; }
	DmeTime_t operator--() { --m_tms; return *this; }
	DmeTime_t operator++( int ) { DmeTime_t t = *this; ++m_tms; return t; } // postfix version..
	DmeTime_t operator--( int ) { DmeTime_t t = *this; --m_tms; return t; } // postfix version..

	// limits, special values and validity

	bool IsValid() const { return m_tms != INT_MIN; }

	static DmeTime_t InvalidTime() { return DmeTime_t( INT_MIN ); }
	static DmeTime_t MinTime() { return DmeTime_t( INT_MIN + 1 ); }
	static DmeTime_t MaxTime() { return DmeTime_t( INT_MAX ); }
	static DmeTime_t MinTimeDelta() { return DmeTime_t( 1 ); }


	// conversions between other time representations

	int GetTenthsOfMS() const { return m_tms; }
	float GetSeconds() const { return m_tms * 0.0001f; }
	void SetTenthsOfMS( int tms ) { m_tms = tms; }

	friend DmeTime_t abs( DmeTime_t t ) { return t.m_tms >= 0 ? t : -t; }

private:

	int m_tms;
};