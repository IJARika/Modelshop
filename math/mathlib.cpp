#include <math.h>

#include "vector.h"

#define PI acos(-1)

inline void SinCos(float x, float* fsin, float* fcos)
{
	*fsin = sin(x);
	*fcos = cos(x);
}

inline float DEG2RAD(float degree)
{
	return degree * PI / 180.0;
}

// Rotate a vector around the Z axis (YAW)
void VectorYawRotate(const Vector& in, float flYaw, Vector& out)
{
	//Assert(s_bMathlibInitialized);
	if (&in == &out)
	{
		Vector tmp;
		tmp = in;
		VectorYawRotate(tmp, flYaw, out);
		return;
	}

	float sy, cy;

	SinCos(DEG2RAD(flYaw), &sy, &cy);

	out.x = in.x * cy - in.y * sy;
	out.y = in.x * sy + in.y * cy;
	out.z = in.z;
}

// these two should go in future math lib
void NormalizeAngles(QAngle& angles)
{
	int i;

	// Normalize angles to -180 to 180 range
	for (i = 0; i < 3; i++)
	{
		if (angles[i] > 180.0)
		{
			angles[i] -= 360.0;
		}
		else if (angles[i] < -180.0)
		{
			angles[i] += 360.0;
		}
	}
}

// needs cleanup
float NormalizeAngle(float ang1, float ang2)
{
	float remainder = fmodf(ang1 - ang2, 360.0);

	// I don't understand this check
	if (ang1 <= ang2)
	{
		if (remainder <= -180.0)
			remainder + 360.0;
	}
	else if (remainder >= 180.0)
	{
		remainder - 360.0;
	}

	return remainder;
}