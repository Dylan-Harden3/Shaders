#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Material.h"
#include "Light.h"

/* So I dont lose points */
#include <limits>
#include <memory>


using namespace std;
using namespace glm;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from
bool OFFLINE = false;

shared_ptr<Camera> camera;
shared_ptr<Program> prog;
shared_ptr<Shape> bunny;
shared_ptr<Shape> teapot;

// vectors to store variable information

vector<shared_ptr<Program>> programs;
int currProgram = 1;

vector<Material> materials;
int currMaterial = 0;

vector<Light> lights;
int currLight = 0;

bool keyToggles[256] = {false}; // only for English keyboards!

// This function is called when a GLFW error occurs
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

// This function is called when the mouse is clicked
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(action == GLFW_PRESS) {
		bool shift = (mods & GLFW_MOD_SHIFT) != 0;
		bool ctrl  = (mods & GLFW_MOD_CONTROL) != 0;
		bool alt   = (mods & GLFW_MOD_ALT) != 0;
		camera->mouseClicked((float)xmouse, (float)ymouse, shift, ctrl, alt);
	}
}

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		camera->mouseMoved((float)xmouse, (float)ymouse);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];

	switch (key) {
		// change shaders
		case 's':
			if (currProgram < programs.size() - 1) { currProgram++; }
			else { currProgram = 0; }
			break;
		case 'S':
			if (currProgram > 0) { currProgram--; }
			else { currProgram = programs.size() - 1; }
			break;
		// change materials
		case 'm':
			if (currMaterial < materials.size() - 1) { currMaterial++; }
			else { currMaterial = 0; }
			break;
		case 'M':
			if (currMaterial > 0) { currMaterial--; }
			else { currMaterial = materials.size() - 1; }
			break;
		// change lights
		case 'l':
			if (currLight < lights.size() - 1) { currLight++; }
			else { currLight = 0;  }
			break;
		case 'L' :
			if (currLight > 0) { currLight--; }
			else { currLight = lights.size() - 1; }
			break;
		// move current light
		case 'x' :
			lights.at(currLight).position.x -= 1.0f;
			break;
		case 'X' :
			lights.at(currLight).position.x += 1.0f;
			break;
		case 'y':
			lights.at(currLight).position.y -= 1.0f;
			break;
		case 'Y':
			lights.at(currLight).position.y += 1.0f;
			break;
	}
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/
static void saveImage(const char *filepath, GLFWwindow *w)
{
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	int rc = stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
	if(rc) {
		cout << "Wrote to " << filepath << endl;
	} else {
		cout << "Couldn't write to " << filepath << endl;
	}
}

// This function is called once to initialize the scene and OpenGL
static void init()
{
	// Initialize time.
	glfwSetTime(0.0);
	
	// Set background color.
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// set up programs with different shaders
	
	// normal shader
	programs.push_back(make_shared<Program>());
	programs.back()->setShaderNames(RESOURCE_DIR + "normal_vert.glsl", RESOURCE_DIR + "normal_frag.glsl");
	
	// Blinn-Phong shader
	programs.push_back(make_shared<Program>());
	programs.back()->setShaderNames(RESOURCE_DIR + "Blinn-Phong_vert.glsl", RESOURCE_DIR + "Blinn-Phong_frag.glsl");

	// Silhouette shader
	programs.push_back(make_shared<Program>());
	programs.back()->setShaderNames(RESOURCE_DIR + "Silhouette_vert.glsl", RESOURCE_DIR + "Silhouette_frag.glsl");

	// Cel shader
	programs.push_back(make_shared<Program>());
	programs.back()->setShaderNames(RESOURCE_DIR + "Cel_vert.glsl", RESOURCE_DIR + "Cel_frag.glsl");
	
	for (int i = 0; i < programs.size() ; i++) {
		programs.at(i)->setVerbose(true);
		programs.at(i)->init();
		programs.at(i)->addAttribute("aPos");
		programs.at(i)->addAttribute("aNor");
		programs.at(i)->addUniform("MV");
		programs.at(i)->addUniform("P");
		programs.at(i)->addUniform("IT");
		programs.at(i)->addUniform("light1Pos");
		programs.at(i)->addUniform("light2Pos");
		programs.at(i)->addUniform("light1Color");
		programs.at(i)->addUniform("light2Color");
		programs.at(i)->addUniform("ka");
		programs.at(i)->addUniform("kd");
		programs.at(i)->addUniform("ks");
		programs.at(i)->addUniform("s");
		programs.at(i)->setVerbose(false);
	}

	camera = make_shared<Camera>();
	camera->setInitDistance(2.0f); // Camera's initial Z translation
	
	// load bunny and teapot
	bunny = make_shared<Shape>();
	bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunny->init();

	teapot = make_shared<Shape>();
	teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapot->init();
	
	GLSL::checkError(GET_FILE_LINE);

	// add materials
	materials.push_back(Material({ 0.2f, 0.2f, 0.2f }, { 0.8f, 0.7f, 0.7f }, { 1.0f, 0.9f, 0.8f }, 200.0f)); // 1st material from lab 7
	materials.push_back(Material({ 0.2f, 0.2f, 0.2f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, 200.0f)); // 2nd blue material
	materials.push_back(Material({ 0.2f, 0.2f, 0.2f }, { 0.5f, 0.5f, 1.0f }, { 0.01f, 0.01f, 0.01f }, 200.0f)); // 3rd gray non shiny

	// add lights
	lights.push_back(Light({ 1.0f, 1.0f, 1.0f }, { 0.8f, 0.8f, 0.8f })); // light 1
	lights.push_back(Light({ -1.0f, 1.0f, 1.0f }, { 0.2f, 0.2f, 0.0f })); // light 2
}

// This function is called every frame to draw the scene.
static void render()
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'z']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	
	double t = glfwGetTime();
	if(!keyToggles[(unsigned)' ']) {
		// Spacebar turns animation on/off
		t = 0.0f;
	}
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	camera->applyViewMatrix(MV);
	
	programs.at(currProgram)->bind();
	glUniform3f(programs.at(currProgram)->getUniform("ka"), materials.at(currMaterial).ka.x, materials.at(currMaterial).ka.y, materials.at(currMaterial).ka.z);
	glUniform3f(programs.at(currProgram)->getUniform("kd"), materials.at(currMaterial).kd.x, materials.at(currMaterial).kd.y, materials.at(currMaterial).kd.z);
	glUniform3f(programs.at(currProgram)->getUniform("ks"), materials.at(currMaterial).ks.x, materials.at(currMaterial).ks.y, materials.at(currMaterial).ks.z);
	glUniform1f(programs.at(currProgram)->getUniform("s"), materials.at(currMaterial).s);
	glUniform3f(programs.at(currProgram)->getUniform("light1Pos"), lights.at(0).position.x, lights.at(0).position.y, lights.at(0).position.z);
	glUniform3f(programs.at(currProgram)->getUniform("light2Pos"), lights.at(1).position.x, lights.at(1).position.y, lights.at(1).position.z);
	glUniform3f(programs.at(currProgram)->getUniform("light1Color"), lights.at(0).color.x, lights.at(0).color.y, lights.at(0).color.z);
	glUniform3f(programs.at(currProgram)->getUniform("light2Color"), lights.at(1).color.x, lights.at(1).color.y, lights.at(1).color.z);

	glUniformMatrix4fv(programs.at(currProgram)->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

	// bunny transformation
	MV->pushMatrix();

	MV->translate(-0.5f, -0.5f, 0.0f);
	MV->scale(0.5f);
	// rotate
	MV->rotate(glfwGetTime(), 0.0f, 1.0f, 0.0f);
	
	glUniformMatrix4fv(programs.at(currProgram)->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

	glm::mat4 IT = glm::transpose(glm::inverse(MV->topMatrix()));
	glUniformMatrix4fv(programs.at(currProgram)->getUniform("IT"), 1, GL_FALSE, glm::value_ptr(IT));

	bunny->draw(programs.at(currProgram));
	MV->popMatrix();

	// teapot transformation
	MV->pushMatrix();
	MV->translate(0.5f, 0.0f, 0.0f);
	MV->scale(0.5f);
	// shear
	glm::mat4 S(1.0f);
	S[0][1] = 0.5f*cos(glfwGetTime());
	MV->multMatrix(S);
	MV->rotate(3.14159, 0.0f, 1.0f, 0.0f);

	glUniformMatrix4fv(programs.at(currProgram)->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	
	IT = glm::transpose(glm::inverse(MV->topMatrix()));
	glUniformMatrix4fv(programs.at(currProgram)->getUniform("IT"), 1, GL_FALSE, glm::value_ptr(IT));

	teapot->draw(programs.at(currProgram));
	MV->popMatrix();

	programs.at(currProgram)->unbind();

	MV->popMatrix();
	P->popMatrix();

	GLSL::checkError(GET_FILE_LINE);
	
	if(OFFLINE) {
		saveImage("output.png", window);
		GLSL::checkError(GET_FILE_LINE);
		glfwSetWindowShouldClose(window, true);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Usage: A3 RESOURCE_DIR" << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Optional argument
	if(argc >= 3) {
		OFFLINE = atoi(argv[2]) != 0;
	}

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(1920, 1080, "DYLAN HARDEN", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	GLSL::checkVersion();
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
