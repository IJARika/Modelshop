#include <pch.h>

#include <core/math/vector.h>

//========
// Vector
//========

// access
float Vector::operator[](int i) const
{
	assert(i >= 0 && 3 > i);
	return ((float*)this)[i];
}

float& Vector::operator[](int i)
{
	assert(i >= 0 && 3 > i);
	return ((float*)this)[i];
}

Vector& Vector::operator=(const Vector& vecIn)
{
	x = vecIn.x;
	y = vecIn.y;
	z = vecIn.z;

	return *this;
}

// math but with copying
Vector Vector::operator+(const Vector& vecIn) const
{
	Vector out;

	out.x = x + vecIn.x;
	out.y = y + vecIn.y;
	out.z = z + vecIn.z;

	return out;
}

Vector Vector::operator-(const Vector& vecIn) const
{
	Vector out;

	out.x = x - vecIn.x;
	out.y = y - vecIn.y;
	out.z = z - vecIn.z;

	return out;
}

Vector Vector::operator*(const Vector& vecIn) const
{
	Vector out;

	out.x = x * vecIn.x;
	out.y = y * vecIn.y;
	out.z = z * vecIn.z;

	return out;
}

Vector Vector::operator/(const Vector& vecIn) const
{
	Vector out;

	out.x = x / vecIn.x;
	out.y = y / vecIn.y;
	out.z = z / vecIn.z;

	return out;
}

Vector Vector::operator*(float inFl) const
{
	Vector out;

	out.x = x * inFl;
	out.y = y * inFl;
	out.z = z * inFl;

	return out;
}

Vector Vector::operator/(float inFl) const
{
	Vector out;

	out.x = x / inFl;
	out.y = y / inFl;
	out.z = z / inFl;

	return out;
}

Vector Vector::operator*(int in) const
{
	Vector out;

	out.x = x * in;
	out.y = y * in;
	out.z = z * in;

	return out;
}

Vector Vector::operator/(int in) const
{
	Vector out;

	out.x = x / in;
	out.y = y / in;
	out.z = z / in;

	return out;
}

const float Vector::Dot(const Vector& a, const Vector& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

const Vector Vector::Normalized() const
{
	Vector normal(*this);
	VectorNormalize(normal);
	return normal;
}

void Vector::NormalizeInPlace()
{
	VectorNormalize(*this);
}

const bool Vector::CompareTolerance(const Vector& vec0, const Vector& vec1, const float tolerance)
{
	Vector vecDelta = vec0 - vec1;
	vecDelta.ABS();

	bool inBounds[3]{};

	inBounds[0] = tolerance > vecDelta[0];
	inBounds[1] = tolerance > vecDelta[1];
	inBounds[2] = tolerance > vecDelta[2];

	return inBounds[0] && inBounds[1] && inBounds[2];
}

const bool Vector::CompareToleranceAbs(const Vector& vec0, const Vector& vec1, const float tolerance)
{
	Vector vec0Abs(vec0);
	Vector vec1Abs(vec1);

	vec0Abs.ABS();
	vec1Abs.ABS();

	return CompareTolerance(vec0Abs, vec1Abs, tolerance);
}


//============
// Quaternion
//============

// access
float Quaternion::operator[](int i) const
{
	assert(i >= 0 && 4 > i);
	return ((float*)this)[i];
}

float& Quaternion::operator[](int i)
{
	assert(i >= 0 && 4 > i);
	return ((float*)this)[i];
}

Quaternion& Quaternion::operator=(const Quaternion& quatIn)
{
	x = quatIn.x;
	y = quatIn.y;
	z = quatIn.z;
	w = quatIn.w;

	return *this;
}


//=============
// RadianEuler
//=============

// access
float RadianEuler::operator[](int i) const
{
	assert(i >= 0 && 3 > i);
	return ((float*)this)[i];
}

float& RadianEuler::operator[](int i)
{
	assert(i >= 0 && 3 > i);
	return ((float*)this)[i];
}

RadianEuler& RadianEuler::operator=(const RadianEuler& angIn)
{
	x = angIn.x;
	y = angIn.y;
	z = angIn.z;

	return *this;
}


//========
// QAngle
//========

// access
float QAngle::operator[](int i) const
{
	assert(i >= 0 && 3 > i);
	return ((float*)this)[i];
}

float& QAngle::operator[](int i)
{
	assert(i >= 0 && 3 > i);
	return ((float*)this)[i];
}

QAngle& QAngle::operator=(const QAngle& qangIn)
{
	x = qangIn.x;
	y = qangIn.y;
	z = qangIn.z;

	return *this;
}

QAngle& QAngle::operator+=(const QAngle& qangIn)
{
	x += qangIn.x;
	y += qangIn.y;
	z += qangIn.z;

	return *this;
}

QAngle& QAngle::operator-=(const QAngle& qangIn)
{
	x -= qangIn.x;
	y -= qangIn.y;
	z -= qangIn.z;

	return *this;
}

QAngle& QAngle::operator*=(float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;

	return *this;
}

QAngle& QAngle::operator/=(float scale)
{
	x /= scale;
	y /= scale;
	z /= scale;

	return *this;
}

QAngle QAngle::operator-() const
{
	QAngle out(-x, -y, -z);
	return out;
}

QAngle QAngle::operator+(const QAngle& qangIn) const
{
	QAngle out;

	out.x = x + qangIn.x;
	out.y = y + qangIn.y;
	out.z = z + qangIn.z;

	return out;
}

QAngle QAngle::operator-(const QAngle& qangIn) const
{
	QAngle out;

	out.x = x - qangIn.x;
	out.y = y - qangIn.y;
	out.z = z - qangIn.z;

	return out;
}

QAngle QAngle::operator*(float inFl) const
{
	QAngle out;

	out.x = x * inFl;
	out.y = y * inFl;
	out.z = z * inFl;

	return out;
}

QAngle QAngle::operator/(float inFl) const
{
	QAngle out;

	out.x = x / inFl;
	out.y = y / inFl;
	out.z = z / inFl;

	return out;
}

// Quaternion
Quaternion::Quaternion(RadianEuler const& angle)
{
	Init();
	AngleQuaternion(angle, *this);
}

// RadianEuler
RadianEuler::RadianEuler(Quaternion const& q)
{
	Init();
	QuaternionAngles(q, *this);
}

RadianEuler::RadianEuler(QAngle const& angles)
{
	Init(
		DEG2RAD(angles.z),
		DEG2RAD(angles.x),
		DEG2RAD(angles.y));
}

QAngle RadianEuler::ToQAngle() const
{
	return QAngle(
		RAD2DEG(y),
		RAD2DEG(z),
		RAD2DEG(x));
}

// QAngle
QAngle::QAngle(RadianEuler const& angIn)
{
	Init(
		RAD2DEG(angIn.y),
		RAD2DEG(angIn.z),
		RAD2DEG(angIn.x));
}

QAngle::QAngle(Quaternion const& quatIn)
{
	Init();
	QuaternionAngles(quatIn, *this);
}

RadianEuler QAngle::ToEuler() const
{
	return RadianEuler(
		DEG2RAD(z),
		DEG2RAD(x),
		DEG2RAD(y));
}