#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "WindowManager.h"
#include "MatrixStack.h"
#include "GLTextureWriter.h"

// used for helper in perspective
#include "glm/glm.hpp"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Terrain.h"
#include "Snowman.h"

using namespace std;
using namespace glm;

int textureIndex = 0;

Snowman * snowman1;
Snowman * snowman2;
Snowman * snowman3;
Snowman * snowman4;
Snowman * snowman5;
Snowman * snowman6;
Snowman * snowman7;
Snowman * snowman8;
Snowman * snowman9;
Snowman * snowman10;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	Terrain * terrain = nullptr;

	// Our shader programs
	// Phong
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> groundProg;

	// texture
	shared_ptr<Texture> textureGrass;
	shared_ptr<Texture> textureSnow;
	shared_ptr<Texture> textureSand;
	shared_ptr<Texture> textureDirt;

	// shader counter for toggle
	int currentShader = 0;

	// Shape to be used (from obj file)
	shared_ptr<Shape> sphere;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//reference to texture FBO
	GLuint frameBuf[2];
	GLuint texBuf[2];
	GLuint depthBuf;

	bool mouseDown = false;

	GLuint IndexBufferID;
	//////////////////////////////////////////////////////////////////////////
	vec3 cameraPosition = { 0.0f, 10.0f, 0.0f };
	vec3 cameraOffset = { 0.0f, 10.0f, 0.0f };
	vec3 lookAtPosition = { 0.0f, 0.0f, 0.0f };

	float light_x_position = 0.0f;

	float cTheta = 0.0f;
	float phi = 0.0f;

	float oldX = 0.0f;
	float oldY = 0.0f;
	float speed = 2.0f;

	float terrainRotation = 0.0f;
	//////////////////////////////////////////////////////////////////////////

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		else if (key == GLFW_KEY_A)
		{
			moveLeft();
		}
		else if (key == GLFW_KEY_D)
		{
			moveRight();
		}
		else if (key == GLFW_KEY_W)
		{
			moveForward();
		}
		else if (key == GLFW_KEY_S)
		{
			moveBackward();
		}
		else if (key == GLFW_KEY_T && action == GLFW_PRESS)
		{
			if (textureIndex < 3)
			{
				textureIndex++;
			}
			else
			{
				textureIndex = 0;
			}
		}
		else if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			terrain->renderSolidTerrain();
		}
		else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		{
			terrain->getNewTerrain();
		}
		else if (key == GLFW_KEY_SPACE)
		{
			light_x_position = 0.0f;

			cameraPosition = { 0.0f, 10.0f, 0.0f };
			cameraOffset = { 0.0f, 10.0f, 0.0f };
			lookAtPosition = { 0.0f, 0.0f, 0.0f };

		}
		else if (key == GLFW_KEY_Q) {
			// move light position along x axis
			light_x_position -= 0.1f;
		}
		else if (key == GLFW_KEY_E) {
			// move light position along x axis
			light_x_position += 0.1f;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			mouseDown = true;
			cout << "X: " << cameraPosition.x << " Y: " << cameraPosition.y << " Z: " << cameraPosition.z << endl;
		}

		if (action == GLFW_RELEASE)
		{
			glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			mouseDown = false;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}
	
	void initTex(const std::string& resourceDirectory)
	{
		textureGrass = make_shared<Texture>();
		textureGrass->setFilename(resourceDirectory + "/grass.jpg");
		textureGrass->init();
		textureGrass->setUnit(0);

		textureSnow = make_shared<Texture>();
		textureSnow->setFilename(resourceDirectory + "/snow.jpg");
		textureSnow->init();
		textureSnow->setUnit(0);

		textureSand = make_shared<Texture>();
		textureSand->setFilename(resourceDirectory + "/sand.jpg");
		textureSand->init();
		textureSand->setUnit(0);

		textureDirt = make_shared<Texture>();
		textureDirt->setFilename(resourceDirectory + "/dirt.jpg");
		textureDirt->init();
		textureDirt->setUnit(0);
	}

	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.55f, 0.70f, 0.88f, 1.0f);

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		initTex(resourceDirectory);

		//////////////////////////////////////////////////////////////////////////
		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/phong_vert.glsl", resourceDirectory + "/phong_frag.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->init();
		prog->addUniform("P");
		prog->addUniform("MV");
		prog->addUniform("view");
		prog->addUniform("light_x_position");

		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");

		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		//////////////////////////////////////////////////////////////////////////
		// Initialize the GLSL program. (ground)
		groundProg = make_shared<Program>();
		groundProg->setVerbose(true);
		groundProg->setShaderNames(resourceDirectory + "/ground_vert.glsl", resourceDirectory + "/ground_frag.glsl");
		if (!groundProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		groundProg->init();
		groundProg->addUniform("P");
		groundProg->addUniform("MV");
		groundProg->addUniform("view");
		groundProg->addUniform("light_x_position");
		groundProg->addAttribute("vertPos");
		groundProg->addAttribute("vertNor");
		groundProg->addAttribute("vertTex");
		groundProg->addUniform("Texture0");
		//////////////////////////////////////////////////////////////////////////
		glGenFramebuffers(1, frameBuf);
		glGenTextures(1, texBuf);
		glGenRenderbuffers(1, &depthBuf);
		createFBO(frameBuf[0], texBuf[0]);

		//set up depth necessary as rendering a mesh that needs depth test
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
	}

	/**** geometry set up for terrain quad *****/
	void initQuad(const std::string& resourceDirectory)
	{
		terrain = new Terrain();
		terrain->generateTerrain();
		createEntities(resourceDirectory);
	}

	/* Helper function to create the framebuffer object and
	associated texture to write to */
	void createFBO(GLuint& fb, GLuint& tex)
	{
		//initialize FBO
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		//set up framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		//set up texture
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "Error setting up frame buffer - exiting" << endl;
			exit(0);
		}
	}

	void moveLeft()
	{
		vec3 forwardVector = glm::normalize(lookAtPosition - cameraPosition);
		vec3 v = glm::cross(vec3(0.0f, 1.0f, 0.0f), forwardVector);
		cameraPosition += v * speed;
		cameraOffset += v * speed;
	}
	void moveRight()
	{
		vec3 forwardVector = glm::normalize(lookAtPosition - cameraPosition);
		vec3 v = glm::cross(forwardVector, vec3(0.0f, 1.0f, 0.0f));
		cameraPosition += v * speed;
		cameraOffset += v * speed;
	}
	void moveForward()
	{
		vec3 forwardVector = glm::normalize(lookAtPosition - cameraPosition);
		cameraPosition += forwardVector * speed;
		cameraOffset += forwardVector * speed;
	}
	void moveBackward()
	{
		vec3 forwardVector = glm::normalize(lookAtPosition - cameraPosition);
		cameraPosition -= forwardVector * speed;
		cameraOffset -= forwardVector * speed;
	}

	void checkCursor()
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		double posX = 0;
		double posY = 0;

		double scale = 15.0;
		double scaleX = 25.0;
		double scaleY = 25.0;

		glfwGetCursorPos(windowManager->getHandle(), &posX, &posY);

		double dx = scale * (posX - width / 2.0) / (scaleX * width) - oldX;
		double dy = scale * (posY - height / 2.0) / (scaleY * height) - oldY;

		oldX = scale * (posX - width / 2.0) / (scaleX * width);
		oldY = scale * (posY - height / 2.0) / (scaleY * height);

		if (mouseDown)
		{
			cTheta += dx;
			phi += -dy;
		}

		if (phi > radians(80.0f)) {
			phi = radians(80.0f);
		}

		if (phi < -radians(80.0f)) {
			phi = -radians(80.0f);
		}

		float radius = 1;
		float lookX = radius * cos(phi) * cos(cTheta);
		float lookY = radius * sin(phi);
		float lookZ = radius * cos(phi) * cos(3.1415 / 2 - cTheta);

		lookAtPosition = vec3(lookX, lookY, lookZ) + cameraOffset;
	}

	void drawGround()
	{
		terrainRotation = glfwGetTime() / 10.0f;
		auto MV = std::make_shared<MatrixStack>();
		MV->pushMatrix();
			//MV->rotate(terrainRotation, vec3(0, 1, 0));
			MV->translate(vec3(-terrain->SIZE / 2, 0, -terrain->SIZE / 2));
			glUniformMatrix4fv(groundProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			if (textureIndex == 0)
			{
				textureGrass->bind(groundProg->getUniform("Texture0"));
			}
			else if (textureIndex == 1)
			{
				textureSnow->bind(groundProg->getUniform("Texture0"));
			}
			else if(textureIndex == 2)
			{
				textureSand->bind(groundProg->getUniform("Texture0"));
			}
			else
			{
				textureDirt->bind(groundProg->getUniform("Texture0"));
			}

			terrain->renderTerrain();
		MV->popMatrix();
	}

	void createEntities(const std::string& resourceDirectory)
	{
		snowman1 = new Snowman(5, 10, 5, 0, terrain, prog, sphere, VertexArrayID, VertexBufferID, IndexBufferID, resourceDirectory);
		snowman2 = new Snowman(20, 10, 50, 1, terrain, prog, sphere, VertexArrayID, VertexBufferID, IndexBufferID, resourceDirectory);
		snowman3 = new Snowman(50, 10, 20, 2, terrain, prog, sphere, VertexArrayID, VertexBufferID, IndexBufferID, resourceDirectory);
		snowman4 = new Snowman(100, 10, 100, 3, terrain, prog, sphere, VertexArrayID, VertexBufferID, IndexBufferID, resourceDirectory);
		snowman5 = new Snowman(200, 10, 70, 0, terrain, prog, sphere, VertexArrayID, VertexBufferID, IndexBufferID, resourceDirectory);
		snowman6 = new Snowman(100, 10, 100, 1, terrain, prog, sphere, VertexArrayID, VertexBufferID, IndexBufferID, resourceDirectory);
		snowman7 = new Snowman(30, 10, 30, 2, terrain, prog, sphere, VertexArrayID, VertexBufferID, IndexBufferID, resourceDirectory);
		snowman8 = new Snowman(100, 10, 230, 3, terrain, prog, sphere, VertexArrayID, VertexBufferID, IndexBufferID, resourceDirectory);
		snowman9 = new Snowman(0, 10, 0, 3, terrain, prog, sphere, VertexArrayID, VertexBufferID, IndexBufferID, resourceDirectory);
		snowman10 = new Snowman(200, 10, 100, 3, terrain, prog, sphere, VertexArrayID, VertexBufferID, IndexBufferID, resourceDirectory);
	}

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto P = std::make_shared<MatrixStack>();

		checkCursor();

		auto view = glm::lookAt(cameraPosition, lookAtPosition, vec3(0.0f, 1.0f, 0.0f));

		// Apply perspective projection.
		P->pushMatrix();
		// original values: 0.01f, 100.0f
		P->perspective(45.0f, aspect, 0.1f, 1000.0f);

		//draw ground
		groundProg->bind();
		glUniformMatrix4fv(groundProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(groundProg->getUniform("view"), 1, GL_FALSE, value_ptr(view));
		glUniform1f(groundProg->getUniform("light_x_position"), light_x_position);
		drawGround();

		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniform1f(prog->getUniform("light_x_position"), light_x_position);

		// globl transforms for 'camera'
		glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE, value_ptr(view));

		//////////////////////////////////////////////////////////////////////////
		snowman1->move();
		snowman2->move();
		snowman3->move();
		snowman4->move();
		snowman5->move();
		snowman6->move();
		snowman7->move();
		snowman8->move();
		snowman9->move();
		snowman10->move();

		snowman1->draw();
		snowman2->draw();
		snowman3->draw();
		snowman4->draw();
		snowman5->draw();
		snowman6->draw();
		snowman7->draw();
		snowman8->draw();
		snowman9->draw();
		snowman10->draw();
	}
};

int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initQuad(resourceDir);

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
