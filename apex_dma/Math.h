#include <math.h>
#include "vector.h"

namespace Math
{
	void NormalizeAngles(Vector& angle);
	double GetFov(const Vector& viewAngle, const Vector& aimAngle);
	double DotProduct(const Vector& v1, const float* v2);
	Vector CalcAngle(const Vector& src, const Vector& dst);
}