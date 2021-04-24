#include <math.h>
#include "vector.h"

struct SVector
{
	float x;
	float y;
	float z;
	SVector(float x1, float y1, float z1)
    {
		x = x1;
		y = y1;
		z = z1;
	}

	SVector(Vector q)
    {
		x = q.x;
		y = q.y;
		z = q.z;
	}
};

namespace Math
{
	void NormalizeAngles(Vector& angle);
	double GetFov(const Vector& viewAngle, const Vector& aimAngle, float distance);
	double GetFov2(const Vector& viewAngle, const Vector& aimAngle);
	double DotProduct(const Vector& v1, const float* v2);
	Vector CalcAngle(const Vector& src, const Vector& dst);
}