#include "Projection.h"

Projection::Projection(GLuint shaderProgram, GLFWwindow *window) {
	this->shaderProgram = shaderProgram;
	this->window = window;

	this->horizontalAngle = 3.14f;
	this->verticalAngle = 0.0f;
	this->position = glm::vec3(0, 0, 5);
	this->projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f); // In world coordinates
    this->viewMatrix = glm::lookAt(
        glm::vec3(0, 2000, 0), // Camera is at (4,3,3), in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1000, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix : an identity matrix (model will be at the origin)
    this->model = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    this->modelViewProjectionMatrix = this->projectionMatrix * this->viewMatrix * this->model; // Remember, matrix multiplication is the other way around

    this->matrix = glGetUniformLocation(this->shaderProgram, "MVP");
}

void Projection::boundMatrixToShaders()
{
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(this->matrix, 1, GL_FALSE, &this->modelViewProjectionMatrix[0][0]);
}

void Projection::handleCamera()
{
	this->matrixFromInput();
	this->modelViewProjectionMatrix = this->projectionMatrix * this->viewMatrix * this->model;
}

void Projection::matrixFromInput()
{
	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(this->window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(this->window, 1024 / 2, 768 / 2);

	// Compute new orientation
	this->horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
	this->verticalAngle += mouseSpeed * float(768 / 2 - ypos);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(this->verticalAngle) * sin(this->horizontalAngle),
		sin(this->verticalAngle),
		cos(this->verticalAngle) * cos(this->horizontalAngle)
	);

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(this->horizontalAngle - 3.14f / 2.0f),
		0,
		cos(this->horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	// Move forward
	if (glfwGetKey(this->window, GLFW_KEY_UP) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey(this->window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey(this->window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey(this->window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
	}

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	this->projectionMatrix = glm::perspective(glm::radians(fov), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	this->viewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}
