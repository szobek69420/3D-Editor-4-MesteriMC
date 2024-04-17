#include "axis.h"
#include "../framework.h"
#include "../Camera/camera.h"

#include <vector>


#define AXIS_LINE_RESOLUTION 100
#define AXIS_LINE_LENGTH 1000.0f

static unsigned int vao=0, vbo=0;
static GPUProgram program;

void Axis::initialize()
{
	//generate axis length
	std::vector<float> vertexData;
	float currentPos = -0.5f * AXIS_LINE_LENGTH;
	float delta = AXIS_LINE_LENGTH / AXIS_LINE_RESOLUTION;
	for (int i = 0; i < AXIS_LINE_RESOLUTION; i++)
	{
		vertexData.push_back(currentPos);
		currentPos += delta;
	}

	//create buffers
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * AXIS_LINE_RESOLUTION, vertexData.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), NULL);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//create shader
	program.createFromFile("./assets/shaders/axis/shader_axis.vag", "./assets/shaders/axis/shader_axis.fag", "fragColour", NULL);
}

void Axis::deinitialize()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void Axis::render(Axis::Direction direction, const Camera& cum,  vec3 center, const vec2& bottomLeft, const vec2& topRight)
{
	glViewport(bottomLeft.x, bottomLeft.y, topRight.x - bottomLeft.x, topRight.y - bottomLeft.y);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1);

	vec3 colour;
	switch (direction)
	{
	case Axis::Direction::DIR_X:
		colour = vec3(1, 0.5f, 0.5f);
		break;

	case Axis::Direction::DIR_Y:
		colour = vec3(0.5f, 1, 0.5f);
		break;

	case Axis::Direction::DIR_Z:
		colour = vec3(0.5f, 0.5f, 1);
		break;

	default:
		return;
	}

	program.Use();
	program.setUniform(colour, "colour");
	program.setUniform(cum.getViewMatrix() * cum.getPerspective((topRight.x-bottomLeft.x)/(topRight.y-bottomLeft.x)), "vp");
	program.setUniform(direction, "direction");
	program.setUniform(center, "center");

	glBindVertexArray(vao);
	
	glDrawArrays(GL_LINE_STRIP, 0, AXIS_LINE_RESOLUTION);

	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_DEPTH_TEST);
}