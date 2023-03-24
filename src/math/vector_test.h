#include <math.h>
#include <cassert>

class Vector
{
public:
	// members
	float x, y, z;

	// checks
	bool IsValid();
	void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f);

	// operators
	float operator[](int i) const; // for if we just want the values
	float& operator[](int i);

	bool operator==(const Vector& v);
	bool operator!=(const Vector& v);

	Vector& operator=(const Vector& iv);

	Vector operator+(const Vector& v) const;
	Vector operator*(float flMult) const;
	Vector& operator*=(float flMult);
	Vector& operator*=(Vector v);

private:

};

bool Vector::IsValid()
{
	return isfinite(x) && isfinite(y) && isfinite(z);
}

void Vector::Init(float ix, float iy, float iz)
{
	x = ix;
	y = iy;
	z = iz;
}

// only so many ways to do something
inline float Vector::operator[](int i) const
{
	assert((i >= 0) && (i < 3));
	return ((float*)this)[i];
}

inline float& Vector::operator[](int i)
{
	assert((i >= 0) && (i < 3));
	return reinterpret_cast<float*>(this)[i];
}

inline bool Vector::operator==(const Vector& v)
{
	return (x == v.x) && (y == v.y) && (z == v.z);
}

inline bool Vector::operator!=(const Vector& v)
{
	return (x != v.x) && (y != v.y) && (z != v.z);
}

inline Vector& Vector::operator=(const Vector& iv)
{
	x = iv.x;
	y = iv.y;
	z = iv.z;

	return *this;
}

inline Vector Vector::operator+(const Vector& v) const
{
	Vector out;

	out.x = x + v.x;
	out.y = y + v.y;
	out.z = z + v.z;

	return out;
}

// only partly understand this
inline Vector Vector::operator*(float flMult) const
{
	Vector out;

	out.x = x * flMult;
	out.y = y * flMult;
	out.z = z * flMult;

	return out;
}

inline Vector operator*(float fl, const Vector& v)
{
	return v * fl;
}

inline Vector& Vector::operator*=(float flMult)
{
	x = x * flMult;
	y = y * flMult;
	z = z * flMult;

	return *this;
}

