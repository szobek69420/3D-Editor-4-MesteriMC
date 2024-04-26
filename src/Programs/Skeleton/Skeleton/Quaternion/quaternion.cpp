#include "quaternion.h"

#include <math.h>

#include "../framework.h"

Quaternion::Quaternion()
{
	this->s = 1;
	this->x = 0;
	this->y = 0;
	this->z = 0;
}

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

Quaternion& Quaternion::operator+=(const Quaternion& _quat)
{
	s += _quat.s;
	x += _quat.x;
	y += _quat.y;
	z += _quat.z;

	return *this;
}

Quaternion Quaternion::operator+(const Quaternion& _quat) const
{
	Quaternion quat2 = *this;
	quat2 += _quat;
	return quat2;
}

Quaternion& Quaternion::operator*=(const Quaternion& _quat)
{
	vec3 v1(this->x, this->y, this->z), v2(_quat.x, _quat.y, _quat.z);
	vec3 temp2 = this->s * v2 + _quat.s * v1 + cross(v1, v2);

	this->s = this->s * _quat.s - dot(v1, v2);
	this->x = temp2.x;
	this->y = temp2.y;
	this->z = temp2.z;

	return *this;
}

Quaternion Quaternion::operator*(const Quaternion& _quat) const
{
	Quaternion temp = *this;
	temp *= _quat;
	return temp;
}

Quaternion operator*(float a, const Quaternion& _quat)
{
	return _quat * a;
}

float Quaternion::angle() const
{
	float cosAngle = this->s;
	float sinAngle = length(vec3(this->x, this->y, this->z));
	
	float angle = 2 * atan2f(sinAngle, cosAngle);
	return angle;
}

vec3 Quaternion::axis() const
{
	vec3 axis(x, y, z);
	
	if (length(axis) > 0.00001f)
		axis = axis / length(axis);

	return axis;
}

float Quaternion::magnitude() const
{
	return sqrtf(s*s+x*x+y*y+z*z);
}

mat4 Quaternion::rotateMatrix() const
{
	vec3 axis = this->axis();
	if (length(axis) < 0.00001f)
		return mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

	mat4 neo = RotationMatrix(this->angle(), axis);
	return neo;
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
Quaternion Quaternion::inverse(const Quaternion& _quat)
{
	Quaternion inv(_quat.s, -_quat.x, -_quat.y, -_quat.z);
	inv /= powf(_quat.magnitude(), 2.0f);
	return inv;
}

Quaternion Quaternion::lerp(const Quaternion& q1, const Quaternion& q2, float t)
{
	float angle = q1.s * q2.s + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
	angle /= q1.magnitude() * q2.magnitude();
	angle = acosf(angle);

	float denominator = sinf(angle);

	Quaternion result = (sinf((1 - t) * angle) / denominator) * q1 + (sinf(t * angle) / denominator) * q2;

	return result;
}

vec3 Quaternion::rotateVector(const vec3& vec, const Quaternion& q)
{
	quat result = q * Quaternion(0, vec.x, vec.y, vec.z) * Quaternion::inverse(q);
	return vec3(result.x, result.y, result.z);
}