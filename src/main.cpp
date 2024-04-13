#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Texture.h"
#include "Item.cpp"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from

shared_ptr<Camera> camera;
shared_ptr<Program> prog;
shared_ptr<Shape> shape; //bunny
shared_ptr<Shape> shape2; //teapot
shared_ptr<Shape> shape3; //cube
shared_ptr<Shape> shape4; // sphere
shared_ptr<Shape> shape5; //SOR

Item* groundPlaneItem;
Item* sunItem;
vector<Item*> items;

//light position buffer
glm::vec3 lights[10];

//light color buffer
glm::vec3 lightColors[10];

//light items
vector<Item*> lightItems;

//posBuf for SOR
vector<float> posBufSOR;

//norBuf for SOR
vector<float> norBufSOR;

//indBuf
vector<unsigned int> indBuf;

shared_ptr<Texture> texture0;

bool keyToggles[256] = {false}; // only for English keyboards!

// This function is called when a GLFW error occurs
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

float randomVal() {
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
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
{	//calculate the forward direction
	//from the forward direction, calculate the left and right direction

	keyToggles[key] = !keyToggles[key];

	glm::vec3 forward(sin(camera->rotations.x), 0, cos(camera->rotations.x));
	glm::vec3 right = glm::cross(forward, glm::vec3(0, 1, 0));
	glm::vec3 left = glm::cross(glm::vec3(0, 1, 0), forward);
	float minZoom = glm::radians(114.0f);
	float maxZoom = glm::radians(4.0f);
	if (key == 'w') {
		camera->translations += glm::normalize(forward) * 0.1f;
	}
	else if (key == 's') {
		camera->translations -= glm::normalize(forward) * 0.1f;
	}
	else if (key == 'd') {
		camera->translations += glm::normalize(right) * 0.1f;
	}
	else if (key == 'a') {
		camera->translations += glm::normalize(left) * 0.1f;
	}
	else if (key == 'z') {
		camera->fovy += .01;
		if (camera->fovy > minZoom) {
			camera->fovy = minZoom;
		}
	}
	else if (key == 'Z') {
		camera->fovy -= .01;
		if (camera->fovy < maxZoom) {
			camera->fovy = maxZoom;
		}
	}
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int calcIndexNum(int numGridPoints, int row, int col) {
	return (row * numGridPoints) + col;
}

// This function is called once to initialize the scene and OpenGL
static void init()
{
	// Initialize time.
	glfwSetTime(0.0);
	
	// Set background color.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	prog = make_shared<Program>();
	prog->setShaderNames(RESOURCE_DIR + "vert.glsl", RESOURCE_DIR + "frag.glsl");
	prog->setVerbose(true);
	prog->init();
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->addAttribute("aTex");
	prog->addUniform("MV");
	prog->addUniform("itMV");
	prog->addUniform("V");
	prog->addUniform("P");
	prog->addUniform("lightPos");
	prog->addUniform("ka");
	prog->addUniform("kd");
	prog->addUniform("ks");
	prog->addUniform("s");
	prog->addUniform("hasTex");
	prog->addUniform("lights");
	prog->addUniform("lightColors");
	prog->addUniform("currLight");
	prog->addUniform("isSOR");
	prog->addUniform("t");
	prog->setVerbose(false);
	
	camera = make_shared<Camera>();
	
	texture0 = make_shared<Texture>();
	texture0->setFilename(RESOURCE_DIR + "grass.jpg");
	texture0->init();
	texture0->setUnit(0);
	texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	
	shape = make_shared<Shape>();
	shape->loadMesh(RESOURCE_DIR + "bunny.obj");
	shape->fitToUnitBox();
	shape->init();

	shape2 = make_shared<Shape>();
	shape2->loadMesh(RESOURCE_DIR + "teapot.obj");
	shape2->fitToUnitBox();
	shape2->init();

	shape3 = make_shared<Shape>();
	shape3->loadMesh(RESOURCE_DIR + "square.obj");
	shape3->fitToUnitBox();
	shape3->init();

	shape4 = make_shared<Shape>();
	shape4->loadMesh(RESOURCE_DIR + "sphere2.obj");
	shape4->fitToUnitBox();
	shape4->init();

	//populate POS/NOR bufs for SOR
	int numGridPoints = 35;
	float xStep = 10.0 / (numGridPoints - 1);
	for (int row = 0; row < numGridPoints; row++) {
		for (int col = 0; col < numGridPoints; col++) {

			float theta = (2 * M_PI / (numGridPoints - 1)) * row;

			//position
			posBufSOR.push_back(col * xStep);
			posBufSOR.push_back(theta);
			posBufSOR.push_back(0.0f);

			//normal
			norBufSOR.push_back(0.0f);
			norBufSOR.push_back(0.0f);
			norBufSOR.push_back(0.0f);



		}
	}

	for (int row = 0; row < numGridPoints - 1; row++) {
		for (int col = 0; col < numGridPoints - 1; col++) {
			//triangle 1
			indBuf.push_back(calcIndexNum(numGridPoints, row, col));
			indBuf.push_back(calcIndexNum(numGridPoints, row, col + 1));
			indBuf.push_back(calcIndexNum(numGridPoints, row + 1, col + 1));

			//triangle 2
			indBuf.push_back(calcIndexNum(numGridPoints, row, col));
			indBuf.push_back(calcIndexNum(numGridPoints, row + 1, col + 1));
			indBuf.push_back(calcIndexNum(numGridPoints, row + 1, col));
		}
	}

	Item* item1 = new Item;
	Item* item2 = new Item;
	Item* item3 = new Item;

	groundPlaneItem = new Item;
	sunItem = new Item;

	groundPlaneItem->ka = glm::vec3(0, 0, 0);
	groundPlaneItem->kd = glm::vec3(0, 0, 0);
	groundPlaneItem->ks = glm::vec3(1.0f, 1.0f, 1.0f);
	groundPlaneItem->s = 100;
	groundPlaneItem->scale = glm::vec3(80, 0.05, 50);
	groundPlaneItem->rotateAngle = glm::radians(-90.0f);
	groundPlaneItem->rotateDirection = glm::vec3(1, 0, 0);
	groundPlaneItem->shape = shape3;
	groundPlaneItem->hasTex = true;

	sunItem->ka = glm::vec3(1, .8, 0);
	sunItem->kd = glm::vec3(0, 0, 0);
	sunItem->ks = glm::vec3(1.0f, 1.0f, 1.0f);
	sunItem->s = 1;
	sunItem->translate = glm::vec3(5, 2.5, 10);
	sunItem->shape = shape4;


	//double for loop that creates all items to be rendered
	//first one traverses thru all rows
	//second one traverse thru cols in that row

	float minYBunny = shape->getMinY();
	float minYTeapot = shape2->getMinY();
	float minYSphere = shape4->getMinY();

	for (int i = 0; i < 20; i+=2) {
		for (int j = -10; j < 10; j+=2) {
			Item* item = new Item;
			float scaleFactor =  1 * randomVal() + 0.5;
			int choice = rand();
			if (choice % 4 == 0) {
				//create bunny
				item->ka = glm::vec3(0.0f, 0.0f, 0.0f);
				item->kd = glm::vec3(randomVal(), randomVal(), randomVal());
				item->ks = glm::vec3(1.0f, 1.0f, 1.0f);
				item->s = 200.0f * randomVal();
				item->translate = glm::vec3(j + randomVal(), 0 - scaleFactor * minYBunny, i + randomVal());
				item->rotateAngle = randomVal() * 2 * M_PI;
				item->rotateDirection = glm::vec3(0, 1, 0);
				item->scale = glm::vec3(scaleFactor, scaleFactor, scaleFactor);
				item->shape = shape;
				item->isBunny = true;
			}
			else if(choice % 4 == 1){
				//create teapot
				item->ka = glm::vec3(0.0f, 0.0f, 0.0f);
				item->kd = glm::vec3(randomVal(), randomVal(), randomVal());
				item->ks = glm::vec3(1.0f, 1.0f, 1.0f);
				item->s = 200.0f * randomVal();
				item->translate = glm::vec3(j + randomVal(), 0 - scaleFactor * minYTeapot, i + randomVal());
				item->rotateAngle = randomVal() * 2 * M_PI;
				item->rotateDirection = glm::vec3(0, 1, 0);
				item->scale = glm::vec3(scaleFactor, scaleFactor, scaleFactor);
				item->shape = shape2;
				item->isTeapot = true;
			}
			else if(choice % 4 == 2) {
				//create sphere
				item->ka = glm::vec3(0.0f, 0.0f, 0.0f);
				item->kd = glm::vec3(randomVal(), randomVal(), randomVal());
				item->ks = glm::vec3(1.0f, 1.0f, 1.0f);
				item->s = 200.0f * randomVal();

				item->translate = glm::vec3(j + randomVal(), 0 - scaleFactor * minYSphere, i + randomVal());
				item->scale = glm::vec3(scaleFactor, scaleFactor, scaleFactor);
				item->shape = shape4;
				item->isSphere = true;
				
			}
			else {
				//create SOR
				item->ka = glm::vec3(0.0f, 0.0f, 0.0f);
				item->kd = glm::vec3(randomVal(), randomVal(), randomVal());
				item->ks = glm::vec3(1.0f, 1.0f, 1.0f);
				item->s = 200.0f * randomVal();

				item->translate = glm::vec3(j + randomVal(), 0, i + randomVal());
				item->scale = glm::vec3(0.2, 0.2, 0.2);
				item->rotateAngle = glm::radians(90.0);
				item->rotateDirection = glm::vec3(0, 0, 1);
				item->isSOR = true;
				item->norBuf = norBufSOR;
				item->posBuf = posBufSOR;
				item->indBuf = indBuf;
			}
			items.push_back(item);
		}
	}	

	//populate light buffers
	for (int i = 0; i < 10; i++) {
		lights[i] = glm::vec3(i * 2.0f - 10, 0.25f, i * 2.0f);
		lightColors[i] = glm::vec3(randomVal(), randomVal(), randomVal());
	}

	//create light items
	for (int i = 0; i < 10; i++) {
		Item* item = new Item;
		item->ka = glm::vec3(0.0f, 0.0f, 0.0f);
		item->kd = glm::vec3(0.0f, 0.0f, 0.0f);
		item->ks = glm::vec3(0.0f, 0.0f, 0.0f);
		item->s = 100.0f;
		item->translate = lights[i];
		item->scale = glm::vec3(0.1, 0.1, 0.1);
		item->shape = shape4;

		lightItems.push_back(item);
	}

	GLSL::checkError(GET_FILE_LINE);
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
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);

	double t = glfwGetTime();

	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	auto V = make_shared<MatrixStack>();

	//light positions in camera space

	
	glViewport(0, 0, width, height);
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);

	prog->bind();

	MV->pushMatrix();
	camera->applyViewMatrix(MV);
	V->pushMatrix();
	camera->applyViewMatrix(V);
	
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, glm::value_ptr(V->topMatrix()));
	glUniform3f(prog->getUniform("lightPos"), 5, 2.5, 10);


	glUniform3fv(prog->getUniform("lights"), 10, glm::value_ptr(lights[0]));
	glUniform3fv(prog->getUniform("lightColors"), 10, glm::value_ptr(lightColors[0]));
	glUniform1i(prog->getUniform("currLight"), -1);
	texture0->bind(prog->getUniform("texture0"));

	//draw sun
	//sunItem->draw(MV, prog);


	//draw lights
	for (int i = 0; i < 10; i++) {
		glUniform1i(prog->getUniform("currLight"), i);
		lightItems[i]->draw(MV, prog);
	}

	glUniform1i(prog->getUniform("currLight"), -1);

	//draw ground plane
	groundPlaneItem->draw(MV, prog);

	//draw items
	for (size_t i = 0; i < items.size(); i++) {
		items[i]->draw(MV, prog);
	}

	MV->popMatrix();
	P->popMatrix();
	
	prog->unbind();
	texture0->unbind();
	
	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "Brian Nguyen", NULL, NULL);
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
