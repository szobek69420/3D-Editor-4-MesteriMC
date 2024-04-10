#include "grid.h"
#include "../framework.h"

static GPUProgram shader;
static unsigned int vao=0, vbo=0;

vec3 Grid::colour = vec3(0.3f, 0.3f, 0.3f);
float Grid::step = 1;

void Grid::initialize()
{
	shader.createFromFile("./assets/shaders/grid/shader_grid.vag", "./assets/shaders/grid/shader_grid.fag", "fragColour", NULL);

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
}
void Grid::deinitialize()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void Grid::render(unsigned int count, const vec2& bottomLeft, const vec2& topRight, const Camera& cum, float start)
{
	glViewport(bottomLeft.x, bottomLeft.y, topRight.x - bottomLeft.x, topRight.y - bottomLeft.y);

	glLineWidth(1);

	glEnable(GL_DEPTH_TEST);

	mat4 vp = cum.getViewMatrix() * PerspectiveMatrix(60, (topRight.x - bottomLeft.x) / (topRight.y - bottomLeft.y), 0.01f, 100.0f);

	shader.Use();
	shader.setUniform(Grid::colour, "colour");
	shader.setUniform(vp, "vp");

	glBindVertexArray(vao);

	//x direction
	shader.setUniform(vec3(69, start, Grid::step), "info");
	glDrawArraysInstanced(GL_LINES, 0, 2, count);

	//z direction
	shader.setUniform(vec3(-69, start, Grid::step), "info");
	glDrawArraysInstanced(GL_LINES, 0, 2, count);

	glBindVertexArray(0);

	glDisable(GL_DEPTH_TEST);
}

void Grid::setStepSize(float stepSize)
{
	Grid::step = stepSize;
}
void Grid::setColour(float r, float g, float b)
{
	Grid::colour = vec3(r, g, b);
}