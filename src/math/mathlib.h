#include <math.h>

#include "vector.h"



inline void SinCos(float x, float* fsin, float* fcos);

inline float DEG2RAD(float degree);

void VectorYawRotate(const Vector& in, float flYaw, Vector& out);

void NormalizeAngles(QAngle& angles);
float NormalizeAngle(float ang1, float ang2);
