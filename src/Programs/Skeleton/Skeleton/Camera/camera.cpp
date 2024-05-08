#include "camera.h"

#include <math.h>

#include "../framework.h"

Camera::Camera()
{
	position = vec3(0);
	yaw = 0;
	pitch = 0;
	refreshViewMatrix();

	clipNear = 0.1f;
	clipFar = 100.0f;
	fov = 60;
}

void Camera::refreshViewMatrix()
{
	vec3 direction = getDirection();
	viewMatrix = LookAtMatrix(position, direction, vec3(0, 1, 0));
}

vec3 Camera::getPosition() const
{
	return position;
}
void Camera::setPosition(const vec3& position)
{
	this->position = position;
}

void Camera::getRotation(float* pitch, float* yaw)
{
	*pitch = this->pitch;
	*yaw = this->yaw;
}
void Camera::setRotation(float pitch, float yaw)
{
	this->pitch = pitch;
	this->yaw = yaw;
}

const mat4& Camera::getViewMatrix() const
{
	return viewMatrix;
}

mat4 Camera::getPerspective(float aspectXY) const
{
	return PerspectiveMatrix(this->fov, aspectXY, this->clipNear, this->clipFar);
}

vec3 Camera::getDirection() const
{
	vec3 direction = vec3(
		-sinf(yaw * 0.01745329252f) * cosf(pitch * 0.01745329252f),
		sinf(pitch * 0.01745329252f),
		-cosf(yaw * 0.01745329252f) * cosf(pitch * 0.01745329252f));
	return direction;
}

vec3 Camera::getRight() const
{
	vec3 right = vec3(-sinf((yaw - 90) * 0.01745329252f), 0, -cosf((yaw - 90) * 0.01745329252f));
	return right;
}

vec3 Camera::getUp() const
{
	vec3 direction=getDirection();
	vec3 right = getRight();
	vec3 up = cross(right, direction);
	return up;
}

float Camera::getFov() const
{
	return this->fov;
}

void Camera::setFov(float fov)
{
	this->fov = fov;
}

float Camera::getClipNear() const
{
	return this->clipNear;
}
void Camera::setClipNear(float clipNear)
{
	this->clipNear = clipNear;
}

float Camera::getClipFar() const
{
	return this->clipFar;
}
void Camera::setClipFar(float clipFar)
{
	this->clipFar = clipFar;
}