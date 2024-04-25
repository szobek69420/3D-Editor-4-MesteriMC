#ifndef QUATERNION_H
#define QUATERNION_H

#include "../framework.h"

class Quaternion {
public:
	float s, x, y, z;

	Quaternion(float s, float x, float y, float z);
	Quaternion(vec4 vec);
	Quaternion(float angleInRads, vec3 axis);

	Quaternion& operator*=(float a);
	Quaternion operator*(float a) const;
	Quaternion& operator/= (float a);
	Quaternion operator/(float a) const;
	Quaternion& operator+=(const Quaternion& quat);
	Quaternion operator+(const Quaternion& quat) const;

	float getAngle() const;

	void normalize();

	static float magnitude(const Quaternion& q);
	static Quaternion lerp(const Quaternion& q1, const Quaternion& q2, float t);
};

Quaternion operator*(float a, const Quaternion& quat);

#endif