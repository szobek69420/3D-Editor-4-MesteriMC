#ifndef CAMERA_H
#define CAMERA_H

#include "../framework.h"

class Camera {
private:
	int currentProjection;
	
	vec3 position;
	float yaw, pitch;

	mat4 viewMatrix;

public:
	Camera();

	void refreshViewMatrix();

	const vec3& getPosition();
	void setPosition(const vec3& position);

	void getRotation(float* pitch, float* yaw);
	void setRotation(float pitch, float yaw);

	const mat4& getViewMatrix() const;

	vec3 getDirection() const;
	vec3 getRight() const;
	vec3 getUp() const;
};

#endif