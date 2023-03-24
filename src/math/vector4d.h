//========= Copyright 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef VECTOR4D_H
#define VECTOR4D_H

#ifdef _WIN32
#pragma once
#endif

#include <math.h>
#include <stdlib.h>		// for rand(). we really need a library!
#define VALVE_RAND_MAX 0x7fff
#include <float.h>
#include <cassert>

#include "vector.h"

//#if !defined( _X360 )
//#include <xmmintrin.h>	// for sse
//#endif
//#include "tier0/basetypes.h"	// For vec_t, put this somewhere else?
//#include "tier0/dbg.h"
//#include "mathlib/math_pfns.h"
//#include "mathlib/vector.h"

// forward declarations
class Vector;
class Vector2D;

//=========================================================
// 4D Vector4D
//=========================================================

class Vector4D					
{
public:
	// Members
	float x, y, z, w;

	// Construction/destruction
	Vector4D(void);
	Vector4D(float X, float Y, float Z, float W);
	Vector4D(const float *pFloat);

	// Initialization
	void Init(float ix=0.0f, float iy=0.0f, float iz=0.0f, float iw=0.0f);
	void Init( const Vector& src, float iw=0.0f );

	// Got any nasty NAN's?
	bool IsValid() const;

	// array access...
	float operator[](int i) const;
	float& operator[](int i);

	// Base address...
	inline float* Base();
	inline float const* Base() const;

	// Cast to Vector and Vector2D...
	Vector& AsVector3D();
	Vector const& AsVector3D() const;

	Vector2D& AsVector2D();
	Vector2D const& AsVector2D() const;

	// Initialization methods
	void Random( float minVal, float maxVal );

	// equality
	bool operator==(const Vector4D& v) const;
	bool operator!=(const Vector4D& v) const;	

	// arithmetic operations
	Vector4D&	operator+=(const Vector4D &v);			
	Vector4D&	operator-=(const Vector4D &v);		
	Vector4D&	operator*=(const Vector4D &v);			
	Vector4D&	operator*=(float s);
	Vector4D&	operator/=(const Vector4D &v);		
	Vector4D&	operator/=(float s);					

	Vector4D	operator-( void ) const;
	Vector4D	operator*( float fl ) const;
	Vector4D	operator/( float fl ) const;
	Vector4D	operator*( const Vector4D& v ) const;
	Vector4D	operator+( const Vector4D& v ) const;
	Vector4D	operator-( const Vector4D& v ) const;

	// negate the Vector4D components
	void	Negate(); 

	// Get the Vector4D's magnitude.
	float	Length() const;

	// Get the Vector4D's magnitude squared.
	float	LengthSqr(void) const;

	// return true if this vector is (0,0,0,0) within tolerance
	bool IsZero( float tolerance = 0.01f ) const
	{
		return (x > -tolerance && x < tolerance &&
			y > -tolerance && y < tolerance &&
			z > -tolerance && z < tolerance &&
			w > -tolerance && w < tolerance);
	}

	// Get the distance from this Vector4D to the other one.
	float	DistTo(const Vector4D &vOther) const;

	// Get the distance from this Vector4D to the other one squared.
	float	DistToSqr(const Vector4D &vOther) const;		

	// Copy
	void	CopyToArray(float* rgfl) const;	

	// Multiply, add, and assign to this (ie: *this = a + b * scalar). This
	// is about 12% faster than the actual Vector4D equation (because it's done per-component
	// rather than per-Vector4D).
	void	MulAdd(Vector4D const& a, Vector4D const& b, float scalar);	

	// Dot product.
	float	Dot(Vector4D const& vOther) const;			

	// No copy constructors allowed if we're in optimal mode
#ifdef VECTOR_NO_SLOW_OPERATIONS
private:
#else
public:
#endif
	Vector4D(Vector4D const& vOther);

	// No assignment operators either...
	Vector4D& operator=( Vector4D const& src );
};

const Vector4D vec4_origin( 0.0f, 0.0f, 0.0f, 0.0f );
const Vector4D vec4_invalid( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX );

//-----------------------------------------------------------------------------
// SSE optimized routines
//-----------------------------------------------------------------------------

//class ALIGN16 Vector4DAligned : public Vector4D
//{
//public:
//	Vector4DAligned(void) {}
//	Vector4DAligned( float X, float Y, float Z, float W );
//
//	inline void Set( float X, float Y, float Z, float W );
//	inline void InitZero( void );
//
//	inline __m128 &AsM128() { return *(__m128*)&x; }
//	inline const __m128 &AsM128() const { return *(const __m128*)&x; } 
//
//private:
//	// No copy constructors allowed if we're in optimal mode
//	Vector4DAligned( Vector4DAligned const& vOther );
//
//	// No assignment operators either...
//	Vector4DAligned& operator=( Vector4DAligned const& src );
//} ALIGN16_POST;

//-----------------------------------------------------------------------------
// Vector4D related operations
//-----------------------------------------------------------------------------

// Vector4D clear
void Vector4DClear( Vector4D& a );

// Copy
void Vector4DCopy( Vector4D const& src, Vector4D& dst );

// Vector4D arithmetic
void Vector4DAdd( Vector4D const& a, Vector4D const& b, Vector4D& result );
void Vector4DSubtract( Vector4D const& a, Vector4D const& b, Vector4D& result );
void Vector4DMultiply( Vector4D const& a, float b, Vector4D& result );
void Vector4DMultiply( Vector4D const& a, Vector4D const& b, Vector4D& result );
void Vector4DDivide( Vector4D const& a, float b, Vector4D& result );
void Vector4DDivide( Vector4D const& a, Vector4D const& b, Vector4D& result );
void Vector4DMA( Vector4D const& start, float s, Vector4D const& dir, Vector4D& result );

// Vector4DAligned arithmetic
//void Vector4DMultiplyAligned( Vector4DAligned const& a, float b, Vector4DAligned& result );


#define Vector4DExpand( v ) (v).x, (v).y, (v).z, (v).w

// Normalization
float Vector4DNormalize( Vector4D& v );

// Length
float Vector4DLength( Vector4D const& v );

// Dot Product
float DotProduct4D(Vector4D const& a, Vector4D const& b);

// Linearly interpolate between two vectors
void Vector4DLerp(Vector4D const& src1, Vector4D const& src2, float t, Vector4D& dest );


//-----------------------------------------------------------------------------
//
// Inlined Vector4D methods
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// constructors
//-----------------------------------------------------------------------------

inline Vector4D::Vector4D(void)									
{ 
#ifdef _DEBUG
	// Initialize to NAN to catch errors
	x = y = z = w = NAN; // float_NAN
#endif
}

inline Vector4D::Vector4D(float X, float Y, float Z, float W )
{ 
	x = X; y = Y; z = Z; w = W;
	assert( IsValid() );
}

inline Vector4D::Vector4D(const float *pFloat)					
{
	assert( pFloat );
	x = pFloat[0]; y = pFloat[1]; z = pFloat[2]; w = pFloat[3];	
	assert( IsValid() );
}


//-----------------------------------------------------------------------------
// copy constructor
//-----------------------------------------------------------------------------

inline Vector4D::Vector4D(const Vector4D &vOther)					
{ 
	assert( vOther.IsValid() );
	x = vOther.x; y = vOther.y; z = vOther.z; w = vOther.w;
}

//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------
inline void Vector4D::Init( float ix, float iy, float iz, float iw )
{ 
	x = ix; y = iy; z = iz;	w = iw;
	assert( IsValid() );
}

inline void Vector4D::Init( const Vector& src, float iw )
{
	x = src.x; y = src.y; z = src.z; w = iw;
	assert( IsValid() );
}


inline void Vector4D::Random( float minVal, float maxVal )
{
	x = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
	y = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
	z = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
	w = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
}

inline void Vector4DClear( Vector4D& a )
{
	a.x = a.y = a.z = a.w = 0.0f;
}

//-----------------------------------------------------------------------------
// assignment
//-----------------------------------------------------------------------------

inline Vector4D& Vector4D::operator=(const Vector4D &vOther)	
{
	assert( vOther.IsValid() );
	x=vOther.x; y=vOther.y; z=vOther.z; w=vOther.w;
	return *this; 
}

//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------

inline float& Vector4D::operator[](int i)
{
	assert( (i >= 0) && (i < 4) );
	return ((float*)this)[i];
}

inline float Vector4D::operator[](int i) const
{
	assert( (i >= 0) && (i < 4) );
	return ((float*)this)[i];
}

//-----------------------------------------------------------------------------
// Cast to Vector and Vector2D...
//-----------------------------------------------------------------------------

inline Vector& Vector4D::AsVector3D()
{
	return *(Vector*)this;
}

inline Vector const& Vector4D::AsVector3D() const
{
	return *(Vector const*)this;
}

inline Vector2D& Vector4D::AsVector2D()
{
	return *(Vector2D*)this;
}

inline Vector2D const& Vector4D::AsVector2D() const
{
	return *(Vector2D const*)this;
}

//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------

inline float* Vector4D::Base()
{
	return (float*)this;
}

inline float const* Vector4D::Base() const
{
	return (float const*)this;
}

//-----------------------------------------------------------------------------
// IsValid?
//-----------------------------------------------------------------------------

inline bool Vector4D::IsValid() const
{
	return isfinite(x) && isfinite(y) && isfinite(z) && isfinite(w);
}

//-----------------------------------------------------------------------------
// comparison
//-----------------------------------------------------------------------------

inline bool Vector4D::operator==( Vector4D const& src ) const
{
	assert( src.IsValid() && IsValid() );
	return (src.x == x) && (src.y == y) && (src.z == z) && (src.w == w);
}

inline bool Vector4D::operator!=( Vector4D const& src ) const
{
	assert( src.IsValid() && IsValid() );
	return (src.x != x) || (src.y != y) || (src.z != z) || (src.w != w);
}


//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------

inline void Vector4DCopy( Vector4D const& src, Vector4D& dst )
{
	assert( src.IsValid() );
	dst.x = src.x;
	dst.y = src.y;
	dst.z = src.z;
	dst.w = src.w;
}

inline void	Vector4D::CopyToArray(float* rgfl) const		
{ 
	assert( IsValid() );
	assert( rgfl );
	rgfl[0] = x; rgfl[1] = y; rgfl[2] = z; rgfl[3] = w;
}

//-----------------------------------------------------------------------------
// standard math operations
//-----------------------------------------------------------------------------

inline void Vector4D::Negate()
{ 
	assert( IsValid() );
	x = -x; y = -y; z = -z; w = -w;
} 

inline Vector4D& Vector4D::operator+=(const Vector4D& v)	
{ 
	assert( IsValid() && v.IsValid() );
	x+=v.x; y+=v.y; z += v.z; w += v.w;	
	return *this;
}

inline Vector4D& Vector4D::operator-=(const Vector4D& v)	
{ 
	assert( IsValid() && v.IsValid() );
	x-=v.x; y-=v.y; z -= v.z; w -= v.w;
	return *this;
}

inline Vector4D& Vector4D::operator*=(float fl)	
{
	x *= fl;
	y *= fl;
	z *= fl;
	w *= fl;
	assert( IsValid() );
	return *this;
}

inline Vector4D& Vector4D::operator*=(Vector4D const& v)	
{ 
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
	assert( IsValid() );
	return *this;
}

inline Vector4D Vector4D::operator-(void) const
{ 
	return Vector4D(-x,-y,-z,-w);				
}

inline Vector4D Vector4D::operator+(const Vector4D& v) const	
{ 
	Vector4D res;
	Vector4DAdd( *this, v, res );
	return res;	
}

inline Vector4D Vector4D::operator-(const Vector4D& v) const	
{ 
	Vector4D res;
	Vector4DSubtract( *this, v, res );
	return res;	
}


inline Vector4D Vector4D::operator*(float fl) const	
{ 
	Vector4D res;
	Vector4DMultiply( *this, fl, res );
	return res;	
}

inline Vector4D Vector4D::operator*(const Vector4D& v) const	
{ 
	Vector4D res;
	Vector4DMultiply( *this, v, res );
	return res;	
}

inline Vector4D Vector4D::operator/(float fl) const	
{ 
	Vector4D res;
	Vector4DDivide( *this, fl, res );
	return res;	
}

inline Vector4D operator*( float fl, const Vector4D& v )	
{ 
	return v * fl; 
}

inline Vector4D& Vector4D::operator/=(float fl)	
{
	assert( fl != 0.0f );
	float oofl = 1.0f / fl;
	x *= oofl;
	y *= oofl;
	z *= oofl;
	w *= oofl;
	assert( IsValid() );
	return *this;
}

inline Vector4D& Vector4D::operator/=(Vector4D const& v)	
{ 
	assert( v.x != 0.0f && v.y != 0.0f && v.z != 0.0f && v.w != 0.0f );
	x /= v.x;
	y /= v.y;
	z /= v.z;
	w /= v.w;
	assert( IsValid() );
	return *this;
}

inline void Vector4DAdd( Vector4D const& a, Vector4D const& b, Vector4D& c )
{
	assert( a.IsValid() && b.IsValid() );
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	c.w = a.w + b.w;
}

inline void Vector4DSubtract( Vector4D const& a, Vector4D const& b, Vector4D& c )
{
	assert( a.IsValid() && b.IsValid() );
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	c.w = a.w - b.w;
}

inline void Vector4DMultiply( Vector4D const& a, float b, Vector4D& c )
{
	assert( a.IsValid() && isfinite(b) );
	c.x = a.x * b;
	c.y = a.y * b;
	c.z = a.z * b;
	c.w = a.w * b;
}

inline void Vector4DMultiply( Vector4D const& a, Vector4D const& b, Vector4D& c )
{
	assert( a.IsValid() && b.IsValid() );
	c.x = a.x * b.x;
	c.y = a.y * b.y;
	c.z = a.z * b.z;
	c.w = a.w * b.w;
}

inline void Vector4DDivide( Vector4D const& a, float b, Vector4D& c )
{
	assert( a.IsValid() );
	assert( b != 0.0f );
	float oob = 1.0f / b;
	c.x = a.x * oob;
	c.y = a.y * oob;
	c.z = a.z * oob;
	c.w = a.w * oob;
}

inline void Vector4DDivide( Vector4D const& a, Vector4D const& b, Vector4D& c )
{
	assert( a.IsValid() );
	assert( (b.x != 0.0f) && (b.y != 0.0f) && (b.z != 0.0f) && (b.w != 0.0f) );
	c.x = a.x / b.x;
	c.y = a.y / b.y;
	c.z = a.z / b.z;
	c.w = a.w / b.w;
}

inline void Vector4DMA( Vector4D const& start, float s, Vector4D const& dir, Vector4D& result )
{
	assert( start.IsValid() && isfinite(s) && dir.IsValid() );
	result.x = start.x + s*dir.x;
	result.y = start.y + s*dir.y;
	result.z = start.z + s*dir.z;
	result.w = start.w + s*dir.w;
}

// FIXME: Remove
// For backwards compatability
inline void	Vector4D::MulAdd(Vector4D const& a, Vector4D const& b, float scalar)
{
	x = a.x + b.x * scalar;
	y = a.y + b.y * scalar;
	z = a.z + b.z * scalar;
	w = a.w + b.w * scalar;
}

inline void Vector4DLerp(const Vector4D& src1, const Vector4D& src2, float t, Vector4D& dest )
{
	dest[0] = src1[0] + (src2[0] - src1[0]) * t;
	dest[1] = src1[1] + (src2[1] - src1[1]) * t;
	dest[2] = src1[2] + (src2[2] - src1[2]) * t;
	dest[3] = src1[3] + (src2[3] - src1[3]) * t;
}

//-----------------------------------------------------------------------------
// dot, cross
//-----------------------------------------------------------------------------

inline float DotProduct4D(const Vector4D& a, const Vector4D& b) 
{ 
	assert( a.IsValid() && b.IsValid() );
	return( a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w ); 
}

// for backwards compatability
inline float Vector4D::Dot( Vector4D const& vOther ) const
{
	return DotProduct4D( *this, vOther );
}


//-----------------------------------------------------------------------------
// length
//-----------------------------------------------------------------------------

inline float Vector4DLength( Vector4D const& v )
{				   
	assert( v.IsValid() );
	return (float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w); // FastSqrt
}

inline float Vector4D::LengthSqr(void) const	
{ 
	assert( IsValid() );
	return (x*x + y*y + z*z + w*w);		
}

inline float Vector4D::Length(void) const	
{
	return Vector4DLength( *this );
}


//-----------------------------------------------------------------------------
// Normalization
//-----------------------------------------------------------------------------

// FIXME: Can't use until we're un-macroed in mathlib.h
inline float Vector4DNormalize( Vector4D& v )
{
	assert( v.IsValid() );
	float l = v.Length();
	if (l != 0.0f)
	{
		v /= l;
	}
	else
	{
		v.x = v.y = v.z = v.w = 0.0f;
	}
	return l;
}

//-----------------------------------------------------------------------------
// Get the distance from this Vector4D to the other one 
//-----------------------------------------------------------------------------

inline float Vector4D::DistTo(const Vector4D &vOther) const
{
	Vector4D delta;
	Vector4DSubtract( *this, vOther, delta );
	return delta.Length();
}

inline float Vector4D::DistToSqr(const Vector4D &vOther) const
{
	Vector4D delta;
	Vector4DSubtract( *this, vOther, delta );
	return delta.LengthSqr();
}


//-----------------------------------------------------------------------------
// Vector4DAligned routines
//-----------------------------------------------------------------------------

//inline Vector4DAligned::Vector4DAligned( float X, float Y, float Z, float W )
//{ 
//	x = X; y = Y; z = Z; w = W;
//	assert( IsValid() );
//}
//
//inline void Vector4DAligned::Set( float X, float Y, float Z, float W )
//{ 
//	x = X; y = Y; z = Z; w = W;
//	assert( IsValid() );
//}
//
//inline void Vector4DAligned::InitZero( void )
//{ 
//#if !defined( _X360 )
//	this->AsM128() = _mm_set1_ps( 0.0f );
//#else
//	this->AsM128() = __vspltisw( 0 );
//#endif
//	assert( IsValid() );
//}
//
//inline void Vector4DMultiplyAligned( Vector4DAligned const& a, Vector4DAligned const& b, Vector4DAligned& c )
//{
//	assert( a.IsValid() && b.IsValid() );
//#if !defined( _X360 )
//	c.x = a.x * b.x;
//	c.y = a.y * b.y;
//	c.z = a.z * b.z;
//	c.w = a.w * b.w;
//#else
//	c.AsM128() = __vmulfp( a.AsM128(), b.AsM128() );
//#endif
//}

//inline void Vector4DWeightMAD( float w, Vector4DAligned const& vInA, Vector4DAligned& vOutA, Vector4DAligned const& vInB, Vector4DAligned& vOutB )
//{
//	assert( vInA.IsValid() && vInB.IsValid() && isfinite(w) );
//
//#if !defined( _X360 )
//	vOutA.x += vInA.x * w;
//	vOutA.y += vInA.y * w;
//	vOutA.z += vInA.z * w;
//	vOutA.w += vInA.w * w;
//
//	vOutB.x += vInB.x * w;
//	vOutB.y += vInB.y * w;
//	vOutB.z += vInB.z * w;
//	vOutB.w += vInB.w * w;
//#else
//	__vector4 temp;
//
//	temp = __lvlx( &w, 0 );
//	temp = __vspltw( temp, 0 );
//
//	vOutA.AsM128() = __vmaddfp( vInA.AsM128(), temp, vOutA.AsM128() );
//	vOutB.AsM128() = __vmaddfp( vInB.AsM128(), temp, vOutB.AsM128() );
//#endif
//}
//
//inline void Vector4DWeightMADSSE( float w, Vector4DAligned const& vInA, Vector4DAligned& vOutA, Vector4DAligned const& vInB, Vector4DAligned& vOutB )
//{
//	assert( vInA.IsValid() && vInB.IsValid() && isfinite(w) );
//
//#if !defined( _X360 )
//	// Replicate scalar float out to 4 components
//	__m128 packed = _mm_set1_ps( w );
//
//	// 4D SSE Vector MAD
//	vOutA.AsM128() = _mm_add_ps( vOutA.AsM128(), _mm_mul_ps( vInA.AsM128(), packed ) );
//	vOutB.AsM128() = _mm_add_ps( vOutB.AsM128(), _mm_mul_ps( vInB.AsM128(), packed ) );
//#else
//	__vector4 temp;
//
//	temp = __lvlx( &w, 0 );
//	temp = __vspltw( temp, 0 );
//
//	vOutA.AsM128() = __vmaddfp( vInA.AsM128(), temp, vOutA.AsM128() );
//	vOutB.AsM128() = __vmaddfp( vInB.AsM128(), temp, vOutB.AsM128() );
//#endif
//}

#endif // VECTOR4D_H

