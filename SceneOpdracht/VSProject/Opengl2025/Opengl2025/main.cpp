#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//forward Declaration
void processInput(GLFWwindow* window);
int init(GLFWwindow* &window);
void createGeometry(GLuint& vao, GLuint& EBO, int& size, int& numIndices);
void createShaders();
void createProgram(GLuint& programID, const char* vertex, const char* fragment);
GLuint loadTexture(const char* path, int comp = 0);
//window callbacks
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool keys[1024];
// util
void loadFile(const char* filename, char*& output);
void renderSkyBox();
void renderTerrain();
void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
unsigned int GeneratePlane(const char* heightmap, unsigned char* &data, GLenum format, int comp, float
	hScale, float xzScale, unsigned int& size, unsigned int& heightmapID);

//Program IDs
GLuint simpleProgram, skyProgram, terrainProgram, modelProgram;

const int WIDTH = 1280, HEIGHT = 720;
float rotationAngle = 0.0f;

//world data
glm::vec3 lightPosition = glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f));
glm::vec3 cameraPosition = glm::vec3(100.0f, 25.0f, 100.0f);
glm::mat4 view, projection;

float lastX, lastY;
bool firstMouse = true;
float camYaw, camPitch;
glm::quat camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0.0f));

GLuint boxVAO, boxEBO;
int boxSize, boxNumIndices;

//Terrain Data
GLuint terrainVAO, terrainIndexCount, heightmapID, heightNormalID;
unsigned char* heightmapTexture;

GLuint dirt, sand, grass, rock, snow;

Model* Cat;
Model* Plant;

int main() {
	GLFWwindow* window;
	int res = init(window);
	if (res != 0) return res;

	stbi_set_flip_vertically_on_load(true);

	createShaders();

	createGeometry(boxVAO, boxEBO, boxSize, boxNumIndices);

	terrainVAO = GeneratePlane("textures/heightmap.png", heightmapTexture, GL_RGBA, 4, 100.0f, 5.0f, terrainIndexCount, heightmapID);
	heightNormalID = loadTexture("textures/heightmap_normal.png");

	GLuint boxTex = loadTexture("textures/brick.png");
	GLuint boxNormal = loadTexture("textures/brick_normal.png");

	dirt = loadTexture("textures/dirt.jpg");
	sand = loadTexture("textures/sand.jpg");
	grass = loadTexture("textures/grass.png", 4);
	rock = loadTexture("textures/rock.jpg");
	snow = loadTexture("textures/snow.jpg");

	Cat = new Model("models/Cat/12221_Cat_v1_l3.obj");
	Plant = new Model("models/Dolphin/10014_dolphin_v2_max2011_it2.obj");

	//tell opengl to create viewport
	glViewport(0, 0, WIDTH, HEIGHT);


	//Matrices!

	view = glm::lookAt(cameraPosition, // Camera position
		glm::vec3(0.0f, 0.0f, 0.0f), // Look at point
		glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
	);

    projection = glm::perspective(glm::radians(45.0f), WIDTH / (float)HEIGHT, 0.1f, 20000.0f);

	
	glEnable(GL_DEPTH_TEST);

	//rendering loop
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LEQUAL);
		renderSkyBox();
		glDepthFunc(GL_LESS);
		renderTerrain();
		float t = glfwGetTime();
		renderModel(Cat, glm::vec3(100,100,100), glm::vec3(-90,t,0),glm::vec3(10,10,10));
		renderModel(Plant, glm::vec3(400, 100, 400), glm::vec3(-90, t, 0), glm::vec3(5, 5, 5));

		processInput(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	return 0;
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	const float mouseSensitivity = 0.9f;
	float x = static_cast<float>(xpos);
	float y = static_cast<float>(ypos);
	if (firstMouse) {
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	float dx = (x - lastX) * mouseSensitivity;
	float dy = (y - lastY) * mouseSensitivity;
	lastX = x;
	lastY = y;

	camYaw -= dx;
	camPitch = glm::clamp(camPitch + dy, -89.0f, 89.0f); // Clamp after adding

	if (camYaw > 180.0f) camYaw -= 360.0f;
	if (camYaw < -180.0f) camYaw += 360.0f;

	camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0.0f));
	glm::vec3 camForward = camQuat * glm::vec3(0, 0, 1);
	glm::vec3 camUp = camQuat * glm::vec3(0, 1, 0);
	view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		keys[key] = true;
	}
	else if (action == GLFW_RELEASE) {
		keys[key] = false;
	}
}
void renderSkyBox() {
	glDepthFunc(GL_LEQUAL); // Allow skybox to be drawn at depth 1.0
	glDisable(GL_CULL_FACE);
	glUseProgram(skyProgram);

	glm::mat4 world = glm::mat4(1.0f);
	world = glm::translate(world, cameraPosition); 
	world = glm::scale(world, glm::vec3(10000.0f,10000.0f,10000.0f));


	glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewNoTranslation));
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(skyProgram, "lightDirection"), 1, glm::value_ptr(lightPosition));
	glUniform3fv(glGetUniformLocation(skyProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	glBindVertexArray(boxVAO);
	glDrawElements(GL_TRIANGLES, boxNumIndices, GL_UNSIGNED_INT, 0);

	glDepthFunc(GL_LESS); // Restore default
	glEnable(GL_CULL_FACE);
}
void renderTerrain() {
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glUseProgram(terrainProgram);


	glm::mat4 world = glm::mat4(1.0f);

	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	float t = glfwGetTime();
	lightPosition = glm::normalize(glm::vec3(glm::sin(t), -0.5f, glm::cos(t)));
	glUniform3fv(glGetUniformLocation(terrainProgram, "lightDirection"), 1, glm::value_ptr(lightPosition));
	glUniform3fv(glGetUniformLocation(terrainProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightNormalID);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, dirt);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, sand);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, grass);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, rock);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, snow);

	glBindVertexArray(terrainVAO);
	glDrawElements(GL_TRIANGLES, terrainIndexCount, GL_UNSIGNED_INT, 0);
}
unsigned int GeneratePlane(const char* heightmap, unsigned char* &data, GLenum format, int comp, float
	hScale, float xzScale, unsigned int& size, unsigned int& heightmapID) {
	int width = 0, height = 0, channels;
	data = nullptr;
	if (heightmap != nullptr) {
		data = stbi_load(heightmap, &width, &height, &channels, comp);
		if (data) {
			glGenTextures(1, &heightmapID);
			glBindTexture(GL_TEXTURE_2D, heightmapID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
				GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	int stride = 8;
	float* vertices = new float[(width * height) * stride];
	unsigned int* indices = new unsigned int[(width - 1) * (height - 1) * 6];
	int index = 0;
	for (int i = 0; i < (width * height); i++) {
		int x = i % width;
		int z = i / width;

		char texHeight = (float)data[i * comp];

		vertices[index++] = x * xzScale;
		vertices[index++] = (texHeight / 255.0f) * hScale;
		vertices[index++] = z * xzScale;
		vertices[index++] = 0;
		vertices[index++] = 1;
		vertices[index++] = 0;
		vertices[index++] = x / (float)(width - 1);
		vertices[index++] = z / (float)(height - 1);
	}
	index = 0;
	for (int i = 0; i < (width - 1) * (height - 1); i++) {
		int x = i % (width - 1);
		int z = i / (width - 1);
		int vertex = z * width + x;
		indices[index++] = vertex;
		indices[index++] = vertex + width + 1;
		indices[index++] = vertex + 1;
		indices[index++] = vertex;
		indices[index++] = vertex + width;
		indices[index++] = vertex + width + 1;
	}
	unsigned int vertSize = (width * height) * stride * sizeof(float);
	size = (width - 1) * (height - 1) * 6;
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
	// vertex information!
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, 0);
	glEnableVertexAttribArray(0);
	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)
		(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
	// uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)
		(sizeof(float) * 6));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	delete[] vertices;
	delete[] indices;
	//stbi_image_free(data);
	return VAO;
}
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	bool camChanged = false;

	if (keys[GLFW_KEY_W]) {
		cameraPosition += camQuat * glm::vec3(0, 0, 1);
		camChanged = true;
	}
	if (keys[GLFW_KEY_S]) {
		cameraPosition += camQuat * glm::vec3(0, 0, -1);
		camChanged = true;
	}
	if (keys[GLFW_KEY_A]) {
		cameraPosition += camQuat * glm::vec3(1, 0, 0);
		camChanged = true;
	}
	if (keys[GLFW_KEY_D]) {
		cameraPosition += camQuat * glm::vec3(-1, 0, 0);
		camChanged = true;
	}
	
	if (camChanged) {
		glm::vec3 camForward = camQuat * glm::vec3(0, 0, 1);
		glm::vec3 camUp = camQuat * glm::vec3(0, 1, 0);
		view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
	}
}

int init (GLFWwindow*& window) {
	//GLFW Init

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//make window

	window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL_2025", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	//register callbacks

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	//load Glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	return 0;
}


void createGeometry(GLuint& vao, GLuint &EBO, int& size, int& numIndices) {
	// Vertices of our triangle!
// need 24 vertices for normal/uv-mapped Cube
	float vertices[] = {
		// positions			//colors			// tex coords // normals		//tangents		//bitagents
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 1.0f,	1.f, 0.f,	0.f, -1.f, 0.f,		-1.f,0.f, 0.f,	0.f, 0.f,1.f,
		0.5f, -0.5f, 0.5f,		1.0f, 1.0f, 0.0f,	1.f, 1.f,	0.f, -1.f, 0.f,		-1.f,0.f, 0.f,	0.f, 0.f,1.f,
		-0.5f, -0.5f, 0.5f,		0.0f, 1.0f, 1.0f,	0.f, 1.f,	0.f, -1.f, 0.f,		-1.f,0.f, 0.f,	0.f, 0.f,1.f,
		-0.5f, -0.5f, -.5f,		1.0f, 1.0f, 1.0f,	0.f, 0.f,	0.f, -1.f, 0.f,		-1.f,0.f, 0.f,	0.f, 0.f,1.f,
		0.5f, 0.5f, -0.5f,		1.0f, 1.0f, 1.0f,	2.f, 0.f,	1.f, 0.f, 0.f,		0.f, -1.f, 0.f,	0.f, 0.f, 1.f,
		0.5f, 0.5f, 0.5f,		1.0f, 1.0f, 1.0f,	2.f, 1.f,	1.f, 0.f, 0.f,		0.f, -1.f, 0.f,	0.f, 0.f, 1.f,
		0.5f, 0.5f, 0.5f,		1.0f, 1.0f, 1.0f,	1.f, 2.f,	0.f, 0.f, 1.f,		1.f, 0.f, 0.f,	0.f, -1.f, 0.f,
		-0.5f, 0.5f, 0.5f,		1.0f, 1.0f, 1.0f,	0.f, 2.f,	0.f, 0.f, 1.f,		1.f, 0.f, 0.f,	0.f, -1.f, 0.f,
		-0.5f, 0.5f, 0.5f,		1.0f, 1.0f, 1.0f,	-1.f, 1.f,	-1.f, 0.f, 0.f,		-1.f,0.f,0.f,	0.f, 0.f, 1.f,
		-0.5f, 0.5f, -.5f,		1.0f, 1.0f, 1.0f,	-1.f, 0.f,	-1.f, 0.f, 0.f,		-1.f,0.f,0.f,	0.f, 0.f, 1.f,
		-0.5f, 0.5f, -.5f,		1.0f, 1.0f, 1.0f,	0.f, -1.f,	0.f, 0.f, -1.f,		1.f,0.f,0.f,	0.f, 1.f, 0.f,
		0.5f, 0.5f, -0.5f,		1.0f, 1.0f, 1.0f,	1.f, -1.f,	0.f, 0.f, -1.f,		1.f,0.f,0.f,	0.f, 1.f, 0.f,
		-0.5f, 0.5f, -.5f,		1.0f, 1.0f, 1.0f,	3.f, 0.f,	0.f, 1.f, 0.f,		1.f,0.f,0.f,	0.f,0.f,1.f,
		-0.5f, 0.5f, 0.5f,		1.0f, 1.0f, 1.0f,	3.f, 1.f,	0.f, 1.f, 0.f,		1.f,0.f,0.f,	0.f,0.f,1.f,
		0.5f, -0.5f, 0.5f,		1.0f, 1.0f, 1.0f,	1.f, 1.f,	0.f, 0.f, 1.f,		1.f, 0.f, 0.f,	0.f, -1.f, 0.f,
		-0.5f, -0.5f, 0.5f,		1.0f, 1.0f, 1.0f,	0.f, 1.f,	0.f, 0.f, 1.f,		1.f, 0.f, 0.f,	0.f, -1.f, 0.f,
		-0.5f, -0.5f, 0.5f,		1.0f, 1.0f, 1.0f,	0.f, 1.f,	-1.f, 0.f, 0.f,		0.f,1.f,0.f,	0.f, 0.f, 1.f,
		-0.5f, -0.5f, -.5f,		1.0f, 1.0f, 1.0f,	0.f, 0.f,	-1.f, 0.f, 0.f,		0.f,1.f,0.f,	0.f, 0.f, 1.f,
		-0.5f, -0.5f, -.5f,		1.0f, 1.0f, 1.0f,	0.f, 0.f,	0.f, 0.f, -1.f,		1.f, 0.f, 0.f,	0.f, 1.f, 0.f,
		0.5f, -0.5f, -0.5f,		1.0f, 1.0f, 1.0f,	1.f, 0.f,	0.f, 0.f, -1.f,		1.f, 0.f, 0.f,	0.f, 1.f, 0.f,
		0.5f, -0.5f, -0.5f,		1.0f, 1.0f, 1.0f,	1.f, 0.f,	1.f, 0.f, 0.f,		0.f, -1.f, 0.f,	0.f, 0.f, 1.f,
		0.5f, -0.5f, 0.5f,		1.0f, 1.0f, 1.0f,	1.f, 1.f,	1.f, 0.f, 0.f,		0.f, -1.f, 0.f,	0.f, 0.f, 1.f,
		0.5f, 0.5f, -0.5f,		1.0f, 1.0f, 1.0f,	2.f, 0.f,	0.f, 1.f, 0.f,		1.f,0.f,0.f,	0.f, 0.f, 1.f,
		0.5f, 0.5f, 0.5f,		1.0f, 1.0f, 1.0f,	2.f, 1.f,	0.f, 1.f, 0.f,		1.f,0.f,0.f,	0.f, 0.f, 1.f,
	};
	unsigned int indices[] = { // note that we start from 0!
		// DOWN
		0, 1, 2, // first triangle
		0, 2, 3, // second triangle
		// BACK
		14, 6, 7, // first triangle
		14, 7, 15, // second triangle
		// RIGHT
		20, 4, 5, // first triangle
		20, 5, 21, // second triangle
		// LEFT
		16, 8, 9, // first triangle
		16, 9, 17, // second triangle
		// FRONT
		18, 10, 11, // first triangle
		18, 11, 19, // second triangle
		// UP
		22, 12, 13, // first triangle
		22, 13, 23, // second triangle
	};

	int stride = (3 + 3 + 2 + 3 + 3 + 3) * sizeof(float);
	size = sizeof(vertices) / stride;
	numIndices = sizeof(indices) / sizeof(int);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0); //DONT FORGET

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1); // DONT FORGET

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2); // DONT FORGET

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, stride, (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3); // DONT FORGET

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_TRUE, stride, (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4); // DONT FORGET

	glVertexAttribPointer(5, 3, GL_FLOAT, GL_TRUE, stride, (void*)(14 * sizeof(float)));
	glEnableVertexAttribArray(5); // DONT FORGET
	
}
void createShaders() {
	glUseProgram(simpleProgram);
	glUniform1i(glGetUniformLocation(simpleProgram, "mainTex"), 0);
	glUniform1i(glGetUniformLocation(simpleProgram, "normalTex"), 1);

	createProgram(simpleProgram, "shaders/simpleVertex.shader", "shaders/simpleFragment.shader");
	createProgram(skyProgram, "shaders/skyVertex.shader", "shaders/skyFragment.shader");
	createProgram(terrainProgram, "shaders/terrainVertex.shader", "shaders/terrainFragment.shader");

	glUseProgram(terrainProgram);
	glUniform1i(glGetUniformLocation(terrainProgram, "mainTex"), 0);
	glUniform1i(glGetUniformLocation(terrainProgram, "normalTex"), 1);

	glUniform1i(glGetUniformLocation(terrainProgram, "dirt"), 2);
	glUniform1i(glGetUniformLocation(terrainProgram, "sand"), 3);
	glUniform1i(glGetUniformLocation(terrainProgram, "grass"), 4);
	glUniform1i(glGetUniformLocation(terrainProgram, "rock"), 5);
	glUniform1i(glGetUniformLocation(terrainProgram, "snow"), 6);

	createProgram(modelProgram, "shaders/model.vs", "shaders/model.fs");
	glUseProgram(modelProgram);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_diffuse1"), 0);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_specular1"), 1);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_normal1"), 2);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_roughness1"), 3);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_ao1"), 4);
}

void createProgram(GLuint& programID, const char* vertex, const char* fragment) {
	char* vertexSrc = nullptr;
	char* fragmentSrc = nullptr;
	loadFile(vertex, vertexSrc);
	loadFile(fragment, fragmentSrc);

	GLuint vertexShaderID, fragmentShaderID;

	vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vertexSrc, nullptr);
	glCompileShader(vertexShaderID);

	int success;
	char infolog[512];
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShaderID, 512, nullptr, infolog);
		std::cerr << "Error compiling vertex shader\n " << infolog << std::endl;
	}

	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fragmentSrc, nullptr);
	glCompileShader(fragmentShaderID);

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShaderID, 512, nullptr, infolog);
		std::cerr << "Error compiling fragment shader\n " << infolog << std::endl;
	}

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(programID, 512, nullptr, infolog);
		std::cerr << "Error linking shader program\n " << infolog << std::endl;
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	delete vertexSrc;
	delete fragmentSrc;
}

void loadFile(const char* filename, char*& output) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		std::streamsize length = file.tellg();
		file.seekg(0, std::ios::beg);

		output = new char[length + 1];
		file.read(output, length);
		output[length] = '\0';

		file.close();
	}
	else {
		std::cerr << "Error opening file: " << filename << std::endl;
		output = nullptr;
	}
}

GLuint loadTexture(const char* path, int comp) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int width, height, numChannels;
	unsigned char* data = stbi_load(path, &width, &height, &numChannels, comp);
	if (data) {
		if (comp != 0) numChannels = comp;
		if(numChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if (numChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Error Loading Texture: " << path << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
	return textureID;
}
void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale) {
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(modelProgram);

	glm::mat4 world = glm::mat4(1.0f);
	world = glm::translate(world, pos);
	world = world * glm::toMat4(glm::quat(rot)); 
	world = glm::scale(world, scale);

	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(modelProgram, "lightDirection"), 1, glm::value_ptr(lightPosition));
	glUniform3fv(glGetUniformLocation(modelProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	model->Draw(modelProgram);
}