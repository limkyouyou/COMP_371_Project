#include <app.h>

App::App()
	: camera(glm::vec3(0.0f, 5.0f, 10.0f))
{
}

int App::run() {
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw: window creation
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "COMP 371 Project", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetWindowUserPointer(window, this); // Pass App instance to GLFW

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Tell GLFW to capture the mouse cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glew: load all OpenGL function pointers
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to create GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	// stbi_set_flip_vertically_on_load(true);

	// Configure global OpenGL state
	glEnable(GL_DEPTH_TEST);

	// Build and compile shaders
	Shader shader("shaders/vertex_shader.vert", "shaders/fragment_shader.frag");

	// Load models
	Model carModel("resources/objects/car/sportcar.017.obj");
	Model roadModel("resources/objects/road/scene4.obj");
	Blimp blimp_1("resources/objects/blimp_1/Aircraft.obj");
	Blimp blimp_2("resources/objects/blimp_1/Aircraft.obj");

	// Set initial position
	carModel.scale = vec3(0.05f, 0.05f, 0.05f);
	carModel.position = vec3(1.0f, 0.3f, 0.0f);
	roadModel.position = vec3(-9.0f, 0.0f, -9.0f); // Manually move the object origin to world origin (object origin is offset)
	blimp_2.angle = pi<float>();	// Make blimp_2 face the opposite direction

	// Update model positions
	blimp_1.update(deltaTime);
	blimp_2.update(deltaTime);

	// Set light properties
	SpotLight blimpLight_1 = {
		blimp_1.position,
		vec3(0.0f, -1.0f, 0.0f),	// light direction (downward)
		12.5f,						// cutOff
		17.5f,						// outerCutOff
		vec3(0.01f),				// ambient
		vec3(1.0f, 1.0f, 0.9f),		// diffuse
		vec3(1.0f),					// specular
		1.0f,						// constant
		0.01,						// linear		0.09, 0.045, 
		0.002f						// quadratic	0.032, 0.0075, 
	};
	SpotLight blimpLight_2 = blimpLight_1;		// Duplicate setting
	blimpLight_2.position = blimp_2.position;

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Input
		processInput(window);

		// Update model positions
		blimp_1.update(deltaTime);
		blimp_2.update(deltaTime);

		// Update spotlight positions from the blimps
		blimpLight_1.position = blimp_1.position;
		blimpLight_2.position = blimp_2.position;

		// render
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Enable the shader program
		shader.use();

		// Update shader
		shader.setVec3("viewPos", camera.Position);
		shader.setVec3("lightPos", vec3(1.2f, 1.0f, 2.0f));
		setSpotLightUniforms(shader, blimpLight_1, 0);
		setSpotLightUniforms(shader, blimpLight_2, 1);

		// View/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		// Draw all models
		carModel.Draw(shader);
		roadModel.Draw(shader);
		blimp_1.Draw(shader);
		blimp_2.Draw(shader);

		// glfw: swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources
	glfwTerminate();
	return 0;
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void App::processInput(GLFWwindow* window) {
	// Retrieve the pointer to the App instance associated with this GLFW window.
	// This allows us to access non-static member variables inside a static callback.
	App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
	if (!app) return;	// Safety check: exit if no App instance is attached to the window

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		app->camera.ProcessKeyboard(FORWARD, app->deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		app->camera.ProcessKeyboard(BACKWARD, app->deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		app->camera.ProcessKeyboard(LEFT, app->deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		app->camera.ProcessKeyboard(RIGHT, app->deltaTime);
}

void App::setSpotLightUniforms(const Shader& shader, const SpotLight& light, int index)
{
	string prefix = "spotLights[" + std::to_string(index) + "].";
	shader.setVec3(prefix + "position", light.position);
	shader.setVec3(prefix + "direction", light.direction);
	shader.setFloat(prefix + "cutOff", cos(radians(light.cutOff)));
	shader.setFloat(prefix + "outerCutOff", cos(radians(light.outerCutOff)));
	shader.setVec3(prefix + "ambient", light.ambient);
	shader.setVec3(prefix + "diffuse", light.diffuse);
	shader.setVec3(prefix + "specular", light.specular);
	shader.setFloat(prefix + "constant", light.constant);
	shader.setFloat(prefix + "linear", light.linear);
	shader.setFloat(prefix + "quadratic", light.quadratic);
}

// Process window resize
void App::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// Process mouse movement
void App::mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
	if (!app) return;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	// If the mouse is moved for the first time, set its last position as the center of the screen
	if (app->firstMouse) {
		app->lastX = xpos;
		app->lastY = ypos;
		app->firstMouse = false;
	}

	float xoffset = xpos - app->lastX;
	float yoffset = app->lastY - ypos;	// reversed since y-coordinates go from bottom to top

	app->lastX = xpos;
	app->lastY = ypos;

	app->camera.ProcessMouseMovement(xoffset, yoffset);
}

// Process mouse scrolls
void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
	if (!app) return;

	app->camera.ProcessMouseScroll(static_cast<float>(yoffset));
}