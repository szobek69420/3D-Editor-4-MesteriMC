#ifndef CAMERA_H
#define CAMERA_H

#include "../framework.h"

class Camera {
private:
	int currentProjection;
	
	vec3 position;
	float yaw, pitch;

	float clipNear, clipFar;
	float fov;

	mat4 viewMatrix;

public:
	Camera();

	void refreshViewMatrix();

	vec3& getPosition();
	void setPosition(const vec3& position);

	void getRotation(float* pitch, float* yaw);
	void setRotation(float pitch, float yaw);

	const mat4& getViewMatrix() const;

	mat4 getPerspective(float aspectXY) const;

	vec3 getDirection() const;
	vec3 getRight() const;
	vec3 getUp() const;

	float getFov() const;
	void setFov(float fov);
};

#endif