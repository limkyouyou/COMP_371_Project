#ifndef APP_H
#define APP_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>
#include <camera.h>
#include <model.h>

#include <iostream>


class App {
public:
	App();

	int run();

private:
	GLFWwindow* window;

	// Settings
	unsigned int SCR_WIDTH = 800;
	unsigned int SCR_HEIGHT = 600;

	// Camera
	Camera camera;
	float lastX = SCR_WIDTH / 2;
	float lastY = SCR_HEIGHT / 2;
	bool firstMouse = true;

	// Timing
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void processInput(GLFWwindow* window);
};


#endif