#ifndef QUATERNION_H
#define QUATERNION_H

#include "../framework.h"

class Quaternion {
public:
	float s, x, y, z;

	Quaternion();
	Quaternion(float s, float x, float y, float z);
	Quaternion(vec4 vec);
	Quaternion(float angleInRads, vec3 axis);

	Quaternion& operator*=(float a);
	Quaternion operator*(float a) const;
	Quaternion& operator/= (float a);
	Quaternion operator/(float a) const;
	Quaternion& operator+=(const Quaternion& _quat);
	Quaternion operator+(const Quaternion& _quat) const;
	Quaternion& operator*=(const Quaternion& _quat);
	Quaternion operator*(const Quaternion& _quat) const;

	float angle() const;
	vec3 axis() const;
	float magnitude() const;
	mat4 rotateMatrix() const;

	void normalize();

	static Quaternion inverse(const Quaternion& _quat);
	static Quaternion lerp(const Quaternion& q1, const Quaternion& q2, float t);
};

Quaternion operator*(float a, const Quaternion& _quat);

typedef Quaternion quat;

#endif