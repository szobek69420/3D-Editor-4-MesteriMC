#include "quaternion.h"

#include <math.h>

#include "../framework.h"

Quaternion::Quaternion(float s, float x, float y, float z)
{
	this->s = s;
	this->x = x;
	this->y = y;
	this->z = z;
}

Quaternion::Quaternion(vec4 vec)
{
	this->s = vec.x;
	this->x = vec.y;
	this->y = vec.z;
	this->z = vec.w;
}

Quaternion::Quaternion(float angleInRads, vec3 axis)
{
	this->s = cosf(0.5f*angleInRads);

	do {
		float temp = length(axis);
		if (temp > 0.00001f)
			axis = axis / temp;
	} while (0);

	float temp = sinf(0.5f * angleInRads);
	this->x = temp * axis.x;
	this->y = temp * axis.y;
	this->z = temp * axis.z;
}

Quaternion& Quaternion::operator*=(float a)
{
	s *= a;
	x *= a;
	y *= a;
	z *= a;
	
	return *this;
}

Quaternion Quaternion::operator*(float a) const
{
	Quaternion quat = *this;
	quat *= a;
	return quat;
}

Quaternion& Quaternion::operator/= (float a)
{
	s /= a;
	x /= a;
	y /= a;
	z /= a;

	return *this;
}

Quaternion Quaternion::operator/(float a) const
{
	Quaternion quat = *this;
	quat /= a;
	return a;
}

Quaternion& Quaternion::operator+=(const Quaternion& quat)
{
	s += quat.s;
	x += quat.x;
	y += quat.y;
	z += quat.z;

	return *this;
}

Quaternion Quaternion::operator+(const Quaternion& quat) const
{
	Quaternion quat2 = *this;
	quat2 += quat;
	return quat2;
}

Quaternion operator*(float a, const Quaternion& quat)
{
	return quat * a;
}

float Quaternion::getAngle() const
{
	float temp = magnitude(*this);
	float cosAngle = this->s / temp;
	float sinAngle = length(vec3(this->x, this->y, this->z)) / temp;
	
	float angle = 2 * atan2f(sinAngle, cosAngle);
	return angle;
}

void Quaternion::normalize()
{
	float length = sqrtf(s * s + x * x + y * y + z * z);
	if (length < 0.00001f)
		return;

	s /= length;
	x /= length;
	y /= length;
	z /= length;
}

//static part
float Quaternion::magnitude(const Quaternion& q)
{
	return sqrtf(q.s * q.s + q.x * q.x + q.y * q.y + q.z * q.z);
}

Quaternion Quaternion::lerp(const Quaternion& q1, const Quaternion& q2, float t)
{
	float angle = q1.s * q2.s + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
	angle /= magnitude(q1) * magnitude(q1);
	angle = acosf(angle);

	float denominator = sinf(angle);

	Quaternion result = (sinf((1 - t) * angle) / denominator) * q1 + (sinf(t * angle) / denominator) * q2;

	return result;
}