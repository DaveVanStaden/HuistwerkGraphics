#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//forward Declaration
void processInput(GLFWwindow* window);
int init(GLFWwindow* &window);
void createGeometry(GLuint& vao, GLuint& EBO, int& size, int& numIndices);
void createShaders();
void createProgram(GLuint& programID, const char* vertex, const char* fragment);
GLuint loadTexture(const char* path);
// util
void loadFile(const char* filename, char*& output);

//Program IDs
GLuint simpleProgram;

const int WIDTH = 1280, HEIGHT = 720;
float rotationAngle = 0.0f;

int main() {
	GLFWwindow* window;
	int res = init(window);
	if (res != 0) return res;

	createShaders();

	GLuint triangleVAO;
	GLuint triangleEBO; 
	int triangleSize;
	int triangleNumIndices;

	createGeometry(triangleVAO, triangleEBO, triangleSize, triangleNumIndices);

	GLuint boxTex = loadTexture("textures/brick.png");
	GLuint boxNormal = loadTexture("textures/brick_normal.png");

	glUseProgram(simpleProgram);
	glUniform1i(glGetUniformLocation(simpleProgram, "mainTex"), 0);
	glUniform1i(glGetUniformLocation(simpleProgram, "normalTex"), 0);



	//tell opengl to create viewport
	glViewport(0, 0, WIDTH, HEIGHT);
	glm::vec3 lightPosition = glm::vec3(0, 2.5f, 5.0f);
	glm::vec3 cameraPosition = glm::vec3(0.0f, 2.5f, -5.0f);

	//Matrices!

	glm::mat4 view = glm::lookAt(cameraPosition, // Camera position
		glm::vec3(0.0f, 0.0f, 0.0f), // Look at point
		glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
	);

    glm::mat4 projection = glm::perspective(glm::radians(35.0f), WIDTH / (float)HEIGHT, 0.1f, 100.0f);

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//rendering loop
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(simpleProgram);

		static float rotationAngle = 0.0f;
		static float lastTime = glfwGetTime();
		float currentTime = glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		rotationAngle += glm::radians(30.0f) * deltaTime;

		glm::mat4 world = glm::mat4(1.0f);
		world = glm::rotate(world, rotationAngle, glm::vec3(0, 1, 0));
		world = glm::scale(world, glm::vec3(1, 1, 1));
		world = glm::translate(world, glm::vec3(0, 0, 0));


		glUniformMatrix4fv(glGetUniformLocation(simpleProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUniform3fv(glGetUniformLocation(simpleProgram, "lightPosition"), 1, glm::value_ptr(lightPosition));
		glUniform3fv(glGetUniformLocation(simpleProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, boxTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, boxNormal);


        glUniform3f(glGetUniformLocation(simpleProgram, "objectColor"), 0.2f, 0.8f, 0.3f); // greenish
		glBindVertexArray(triangleVAO);
		//glDrawArrays(GL_TRIANGLES, 0, triangleSize);
		glDrawElements(GL_TRIANGLES, triangleNumIndices, GL_UNSIGNED_INT, 0);

		processInput(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	return 0;
}
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
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
	createProgram(simpleProgram, "shaders/simpleVertex.shader", "shaders/simpleFragment.shader");
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

GLuint loadTexture(const char* path) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, numChannels;
	unsigned char* data = stbi_load(path, &width, &height, &numChannels, 0);
	if (data) {
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