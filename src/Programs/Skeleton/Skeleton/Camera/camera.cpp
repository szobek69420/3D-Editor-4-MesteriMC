#include "camera.h"

#include <math.h>

Camera::Camera()
{
	position = vec3(0);
	yaw = 0;
	pitch = 0;

	refreshViewMatrix();
}

void Camera::refreshViewMatrix()
{
	vec3 direction = getDirection();
	viewMatrix = LookAtMatrix(position, direction, vec3(0, 1, 0));
}

vec3& Camera::getPosition()
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


//static part
float Camera::fov = 60;

float Camera::getFov()
{
	return Camera::fov;
}

void Camera::setFov(float fov)
{
	Camera::fov = fov;
}