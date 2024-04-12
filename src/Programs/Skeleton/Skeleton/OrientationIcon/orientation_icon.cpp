#include "orientation_icon.h"

#include "../framework.h"
#include "../System/system.h"

static GPUProgram program;
static unsigned int vao, vbo;
static const mat4 projection = OrthoMatrix(-1, 1, -1, 1, -1, 1);

static float vboData[] = {
	1,0,0,	1,0,0,
	0,0,0,	1,0,0,
	0,1,0,	0,1,0,
	0,0,0,	0,1,0,
	0,0,1,	0,0,1,
	0,0,0,	0,0,1
};

void OrientationIcon::initialize()
{
	program.createFromFile(
		"./assets/shaders/orientation/shader_orientation.vag",
		"./assets/shaders/orientation/shader_orientation.fag",
		"fragColour",
		NULL);

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vboData), vboData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));

	glBindVertexArray(0);
}

void OrientationIcon::deinitialize()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void OrientationIcon::render(const Camera& cum, vec2 bottomLeft, vec2 topRight)
{
	int windowWidth, windowHeight;
	System::getWindowSize(&windowWidth, &windowHeight);

	glViewport(bottomLeft.x, bottomLeft.y, topRight.x - bottomLeft.x, topRight.y - bottomLeft.y);
	glLineWidth(5);

	mat4 mvp = TranslateMatrix(cum.getPosition()) * cum.getViewMatrix() * projection;

	program.Use();
	program.setUniform(mvp, "mvp");

	glBindVertexArray(vao);

	glDrawArrays(GL_LINES, 0, 6);

	glBindVertexArray(0);
	glUseProgram(0);

	glViewport(0, 0, windowWidth, windowHeight);
}