#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include "OpenGL.h"

constexpr auto speed = 3.0f;
constexpr auto mouseSpeed = 0.005f;
constexpr auto fov = 45.0f;

class Projection
{
public:
	Projection(GLuint shaderProgram, GLFWwindow *window);
	~Projection();

	void boundMatrixToShaders();
	void handleCamera();

private:
	void matrixFromInput();

private:
	GLuint shaderProgram;
	GLFWwindow* window;

	glm::vec3 position;
	float horizontalAngle;
	float verticalAngle;
	glm::mat4 model;

public:
	GLuint matrix;
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 modelViewProjectionMatrix; // model view projection matrix
};


#endif