#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "WindowManager.h"
#include "MatrixStack.h"

// used for helper in perspective
#include "glm/glm.hpp"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

// number of triangles
const int NUM_OF_TRIANGLES = 50;

// number of levels
const int NUM_OF_LEVELS = 5;

float arm_rotate = 45.0f;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader programs
	// Phong
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> groundProg;
	std::shared_ptr<Program> mirrorProg;

	// shader counter for toggle
	int currentShader = 0;

	// Shape to be used (from obj file)
	shared_ptr<Shape> bunny;
	shared_ptr<Shape> sphere;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	//reference to texture FBO
	GLuint frameBuf[2];
	GLuint texBuf[2];
	GLuint depthBuf;

	bool FirstTime = true;
	bool Moving = false;
	bool mouseDown = false;

	GLuint IndexBufferID;
	//////////////////////////////////////////////////////////////////////////
	vec3 cameraPosition = { 0.0f, 0.0f, 0.0f };
	vec3 cameraOffset = { 0.0f, 0.0f, 0.0f };
	vec3 lookAtPosition = { 0.0f, 0.0f, 0.0f };
	vec3 mirrorPosition = { 5.0, 1.0, 3.0 };

	float object_x_rotation = 0.0f;
	float object_y_rotation = 0.0f;
	float object_z_rotation = 0.0f;

	float light_x_position = -2.0f;
	float light_y_position = 2.0f;
	float light_z_position = 2.0f;

	float cTheta = 0.0f;
	float phi = 0.0f;

	float oldX = 0.0f;
	float oldY = 0.0f;
	float speed = 0.2f;
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
		else if (key == GLFW_KEY_SPACE)
		{
			object_x_rotation = 0.0f;
			object_y_rotation = 0.0f;
			object_z_rotation = 0.0f;

			light_x_position = -2.0f;
			light_y_position = 2.0f;
			light_z_position = 2.0f;

			cameraPosition = { 0.0f, 0.0f, 0.0f };
			cameraOffset = { 0.0f, 0.0f, 0.0f };
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
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			mouseDown = true;
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
			Moving = true;
		}

		if (action == GLFW_RELEASE)
		{
			Moving = false;
			mouseDown = false;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	//////////////////////////////////////////////////////////////////////////
	// Snowman
	// create a cylinder divided into triangles
	void createVBO(GLfloat* array) {
		// top of circle
		float newX;
		float newZ;
		// center of circle
		float centerX = 0.0f;
		float centerZ = 0.0f;

		// angle to rotate
		float angle = glm::radians(360.0 / NUM_OF_TRIANGLES);

		for (int y = 0; y < NUM_OF_LEVELS; y++) {
			newX = 0.0f;
			newZ = 0.5f;
			for (int i = 0; i < NUM_OF_TRIANGLES; i++) {
				array[3 * i + NUM_OF_TRIANGLES * y * 3] = newX; // x
				array[3 * i + 1 + NUM_OF_TRIANGLES * y * 3] = y / float(NUM_OF_LEVELS); // y
				array[3 * i + 2 + NUM_OF_TRIANGLES * y * 3] = newZ; // z

				float x1 = newX - centerX;
				float z1 = newZ - centerZ;

				// rotate point
				float x2 = x1 * cos(angle) - z1 * sin(angle);
				float z2 = x1 * sin(angle) + z1 * cos(angle);

				newX = x2 + centerX;
				newZ = z2 + centerZ;
			}
		}
	}

	void createIBO(GLuint* array) {
		for (int j = 0; j < NUM_OF_LEVELS; j++)
		{
			// last level
			if (j == NUM_OF_LEVELS - 1) {
				return;
			}

			for (int i = 0; i < NUM_OF_TRIANGLES; i++)
			{
				// last index in each level
				if (i == NUM_OF_TRIANGLES - 1) {
					// Triangles Up
					// i, i + 1, i + NUM_OF_TRIANGLES + 1
					int index = (i + j * NUM_OF_TRIANGLES) * 6;
					int k = i + (j * NUM_OF_TRIANGLES);
					array[index] = k;
					array[index + 1] = k + 1 - NUM_OF_TRIANGLES;
					array[index + 2] = k + 1;

					// Triangles Down
					// i, i + NUM_OF_TRIANGLES, i + NUM_OF_TRIANGLES + 1
					array[index + 3] = k;
					array[index + 4] = k + NUM_OF_TRIANGLES;
					array[index + 5] = k + 1;
					break;
				}

				// Triangles Up
				// i, i + 1, i + NUM_OF_TRIANGLES + 1
				int index = (i + j * NUM_OF_TRIANGLES) * 6;
				int k = i + (j * NUM_OF_TRIANGLES);
				array[index] = k;
				array[index + 1] = k + 1;
				array[index + 2] = k + NUM_OF_TRIANGLES + 1;

				// Triangles Down
				// i, i + NUM_OF_TRIANGLES, i + NUM_OF_TRIANGLES + 1
				array[index + 3] = k;
				array[index + 4] = k + NUM_OF_TRIANGLES;
				array[index + 5] = k + NUM_OF_TRIANGLES + 1;
			}
		}

	}

	void initSnowman(const std::string& resourceDirectory)
	{
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);

		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat g_vertex_buffer_data[(NUM_OF_TRIANGLES * NUM_OF_LEVELS) * 3];
		createVBO(g_vertex_buffer_data);

		GLuint g_index_buffer_data[NUM_OF_TRIANGLES * NUM_OF_LEVELS * 2 * 3];
		createIBO(g_index_buffer_data);

		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Create and bind IBO
		glGenBuffers(1, &IndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_index_buffer_data), g_index_buffer_data, GL_DYNAMIC_DRAW);

		glBindVertexArray(0);

		///////////////////////////////////////////////////////////////////////////////////////
		// Initialize mesh.
		sphere = make_shared<Shape>();
		sphere->loadMesh(resourceDirectory + "/sphere.obj");
		sphere->resize();
		sphere->init();
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
			
		groundProg->addAttribute("vertPos");
		//////////////////////////////////////////////////////////////////////////
		glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		//create two frame buffer objects to toggle between
		glGenFramebuffers(2, frameBuf);
		glGenTextures(2, texBuf);
		glGenRenderbuffers(1, &depthBuf);
		createFBO(frameBuf[0], texBuf[0]);

		//set up depth necessary as rendering a mesh that needs depth test
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

		//more FBO set up
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);

		//create another FBO so we can swap back and forth
		createFBO(frameBuf[1], texBuf[1]);

		/////////////////////////////////////////////////////////////
		// Mirror
		mirrorProg = make_shared<Program>();
		mirrorProg->setVerbose(true);
		mirrorProg->setShaderNames(resourceDirectory + "/mirror_vert.glsl", resourceDirectory + "/mirror_frag.glsl");
		if (!mirrorProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		mirrorProg->init();
		mirrorProg->addUniform("P");
		mirrorProg->addUniform("MV");
		mirrorProg->addUniform("view");

		mirrorProg->addAttribute("vertPos");
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh.
		bunny = make_shared<Shape>();
		bunny->loadMesh(resourceDirectory + "/bunny.obj");
		bunny->resize();
		bunny->init();
		initQuad();
	}

	/**** geometry set up for a quad *****/
	void initQuad()
	{
		//now set up a simple quad for rendering FBO
		glGenVertexArrays(1, &quad_VertexArrayID);
		glBindVertexArray(quad_VertexArrayID);

		static const GLfloat g_quad_vertex_buffer_data[] =
		{
			-1.0f, -1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,
		};

		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
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

	// To complete image processing on the specificed texture
	// Right now just draws large quad to the screen that is texture mapped
	// with the prior scene image - next lab we will process
	void ProcessImage(GLuint inTex)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, inTex);

		// example applying of 'drawing' the FBO texture - change shaders
		mirrorProg->bind();
		glUniform1i(mirrorProg->getUniform("texBuf"), 0);
		glUniform2f(mirrorProg->getUniform("dir"), -1, 0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(0);
		mirrorProg->unbind();
	}

	void setMaterial(int i) {
		int index = i;
		if (index > 3) index = 0;
		switch (index) {
		case 0: // shiny blue plastic
			glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
			glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
			glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);
			glUniform1f(prog->getUniform("shine"), 120.0);
			break;
		case 1: // flat grey
			glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
			glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);
			glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);
			glUniform1f(prog->getUniform("shine"), 4.0);
			break;
		case 2: // brass
			glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
			glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
			glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
			glUniform1f(prog->getUniform("shine"), 27.9);
			break;
		case 3: // shiny bright pink
			glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
			glUniform3f(prog->getUniform("MatDif"), 0.956, 0.248, 0.419);
			glUniform3f(prog->getUniform("MatSpec"), 0.5, 0.5, 0.5);
			glUniform1f(prog->getUniform("shine"), 80.0);
			break;
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

		cTheta += dx;
		phi += -dy;

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

	void drawMirror()
	{
		auto MV = std::make_shared<MatrixStack>();
		glBindVertexArray(quad_VertexArrayID);
		MV->pushMatrix();
			MV->translate(mirrorPosition);
			MV->scale(vec3(3.0, 2.0, 1.0));
			
			glUniformMatrix4fv(mirrorProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(0);
		MV->popMatrix();
	}

	void drawGround()
	{
		auto MV = std::make_shared<MatrixStack>();
		glBindVertexArray(quad_VertexArrayID);
		MV->pushMatrix();
			MV->translate(vec3(0, -1.0, 0));
			MV->rotate(radians(90.0f), vec3(1, 0, 0));
			MV->scale(30.0f);
			
			glUniformMatrix4fv(groundProg->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(0);
		MV->popMatrix();
	}

	void drawBunny(float x, float y, float z, int color)
	{
		auto MV = std::make_shared<MatrixStack>();
		MV->pushMatrix();
			MV->translate(vec3(x, y, z));
			MV->rotate(3.14f, vec3(0, 1, 0));
			setMaterial(color % 4);
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
			bunny->draw(prog);
		MV->popMatrix();
	}

	void drawSnowman(float x, float y, float z, int color)
	{
		auto MV = std::make_shared<MatrixStack>();
		
		// arm rotation by time
		arm_rotate = sin(glfwGetTime());

		// Global
		MV->pushMatrix();
			MV->loadIdentity();
			MV->translate(vec3(x, y + 0.25, z));
			MV->scale(vec3(0.25f, 0.25f, 0.25f));
			setMaterial(color % 4);
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		//////////////////////////////////////////////////////////////////////////

		// Body Middle
			MV->pushMatrix();
				MV->translate(vec3(0.0f, 0.0f, 0.0f));
				MV->rotate(0, vec3(0.0f, 0.0f, 1.0f));
				MV->scale(vec3(1.5f, 1.5f, 1.5f));
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				sphere->draw(prog);

		//////////////////////////////////////////////////////////////////////////

		// Body Bottom
				MV->pushMatrix();
					MV->translate(vec3(0.0f, -2.0f, 0.0f));
					MV->rotate(0, vec3(0.0f, 0.0f, 1.0f));
					MV->scale(vec3(1.5f, 1.5f, 1.5f));
					glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
					sphere->draw(prog);

		//////////////////////////////////////////////////////////////////////////

		// Head
				MV->popMatrix();
				MV->pushMatrix();
					MV->translate(vec3(0.0f, 1.5f, 0.0f));
					MV->rotate(0, vec3(0.0f, 0.0f, 1.0f));
					MV->scale(vec3(0.75f, 0.75f, 0.75f));
					glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
					sphere->draw(prog);

		//////////////////////////////////////////////////////////////////////////
	
		// Hat Base
		glBindVertexArray(VertexArrayID);
					MV->pushMatrix();
						MV->translate(vec3(0.0f, 0.5f, 0.0f));
						MV->rotate(0, vec3(1.0f, 0.0f, 0.0f));
						MV->scale(vec3(2.0f, 0.75f, 2.0f));
						glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
						glDrawElements(GL_TRIANGLES, NUM_OF_TRIANGLES * NUM_OF_LEVELS * 6, GL_UNSIGNED_INT, nullptr);

		//////////////////////////////////////////////////////////////////////////

		// Hat Top
		glBindVertexArray(VertexArrayID);
						MV->pushMatrix();
							MV->translate(vec3(0.0f, 0.0f, 0.0f));
							MV->rotate(0, vec3(1.0f, 0.0f, 0.0f));
							MV->scale(vec3(0.5f, 2.5f, 0.5f));
							glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
							glDrawElements(GL_TRIANGLES, NUM_OF_TRIANGLES * NUM_OF_LEVELS * 6, GL_UNSIGNED_INT, nullptr);
						MV->popMatrix();

		//////////////////////////////////////////////////////////////////////////

		// Arm Left
					MV->popMatrix();
				MV->popMatrix();
				MV->pushMatrix();
					MV->translate(vec3(-1.0f, 0.5f, 0.0f));
					MV->rotate(arm_rotate, vec3(0.0f, 0.0f, 1.0f));
					MV->scale(vec3(0.5f, 0.5f, 0.5f));
					glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
					sphere->draw(prog);
					MV->pushMatrix();
						MV->translate(vec3(-1.0f, 0.0f, 0.0f));
						MV->rotate(0, vec3(0.0f, 0.0f, 1.0f));
						MV->scale(vec3(1.0f, 0.75f, 0.75f));
						glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
						sphere->draw(prog);
						MV->pushMatrix();
							MV->translate(vec3(-1.0f, 0.0f, 0.0f));
							MV->rotate(0, vec3(0.0f, 0.0f, 1.0f));
							MV->scale(vec3(1.0f, 0.75f, 0.75f));
							glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
							sphere->draw(prog);
						MV->popMatrix();
					MV->popMatrix();
				MV->popMatrix();
		//////////////////////////////////////////////////////////////////////////

		// Arm Right
				MV->pushMatrix();
				MV->translate(vec3(1.0f, 0.5f, 0.0f));
				MV->rotate(-arm_rotate, vec3(0.0f, 0.0f, 1.0f));
				MV->scale(vec3(0.5f, 0.5f, 0.5f));
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				sphere->draw(prog);
				MV->pushMatrix();
					MV->translate(vec3(1.0f, 0.0f, 0.0f));
					MV->rotate(0, vec3(0.0f, 0.0f, 1.0f));
					MV->scale(vec3(1.0f, 0.75f, 0.75f));
					glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
					sphere->draw(prog);
					MV->pushMatrix();
						MV->translate(vec3(1.0f, 0.0f, 0.0f));
						MV->rotate(0, vec3(0.0f, 0.0f, 1.0f));
						MV->scale(vec3(1.0f, 0.75f, 0.75f));
						glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
						sphere->draw(prog);
					MV->popMatrix();
				MV->popMatrix();
			MV->popMatrix();
		MV->popMatrix();
	}

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		if (Moving)
		{
			//set up to render to buffer
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[0]);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto P = std::make_shared<MatrixStack>();

		checkCursor();

		auto view = glm::lookAt(cameraPosition, lookAtPosition, vec3(0.0f, 1.0f, 0.0f));

		// Apply perspective projection.
		P->pushMatrix();
		P->perspective(45.0f, aspect, 0.01f, 100.0f);

		groundProg->bind();
		glUniformMatrix4fv(groundProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(groundProg->getUniform("view"), 1, GL_FALSE, value_ptr(view));
		drawGround();

		mirrorProg->bind();
		glUniformMatrix4fv(mirrorProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(mirrorProg->getUniform("view"), 1, GL_FALSE, value_ptr(view));
		drawMirror();

		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniform1f(prog->getUniform("light_x_position"), light_x_position);

		// globl transforms for 'camera'
		glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE, value_ptr(view));

		//////////////////////////////////////////////////////////////////////////
		drawBunny(0, 0, -10, 0);
		drawBunny(2, 0, 5, 1);
		drawBunny(-5, 0, 5, 2);
		drawBunny(-15, 0, -5, 3);
		drawBunny(-15, 0, 10, 4);
		drawBunny(7, 0, -10, 5);
		drawBunny(10, 0, -10, 6);
		drawBunny(10, 0, 10, 7);
		drawBunny(-5, 0, 10, 8);
		drawBunny(-5, 0, 10, 9);
		//////////////////////////////////////////////////////////////////////////
		drawSnowman(5, 0, -8, 0);
		drawSnowman(9, 0, 6, 1);
		drawSnowman(-4, 0, 3, 2);
		drawSnowman(-18, 0, -2, 3);
		drawSnowman(0, 0, 10, 4);
		drawSnowman(7, 0, -7, 5);
		drawSnowman(16, 0, -7, 6);
		drawSnowman(10, 0, -4, 7);
		drawSnowman(-1, 0, 5, 8);
		drawSnowman(-16, 0, 5, 9);
		//////////////////////////////////////////////////////////////////////////
		if (Moving)
		{
			for (int i = 0; i < 3; i++)
			{
				//set up framebuffer
				glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[(i + 1) % 2]);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				//set up texture
				ProcessImage(texBuf[i % 2]);
			}

			/* now draw the actual output */
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ProcessImage(texBuf[1]);
		}
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
	application->initGeom(resourceDir);
	application->initSnowman(resourceDir);

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
