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

#include "Snowman.h"

using namespace std;
using namespace glm;

Snowman::Snowman(float _playerPositionX, float _playerPositionY, float _playerPositionZ, int _color,
	Terrain * _terrain, std::shared_ptr<Program> _prog, std::shared_ptr<Shape> _sphere, 
	GLuint _VertexArrayID, GLuint _VertexBufferID, GLuint _IndexBufferID, const std::string& resourceDirectory)
{
	positionX = _playerPositionX;
	playerPositionY = _playerPositionY;
	positionZ = _playerPositionZ;
	color = _color;
	terrain = _terrain;
	prog = _prog;
	sphere = _sphere;
	VertexArrayID = _VertexArrayID;
	VertexBufferID = _VertexBufferID;
	IndexBufferID = _IndexBufferID;

	speedX = (rand() % 10 + 1) * 0.1f;
	speedZ = (rand() % 10 + 1) * 0.1f;

	initSnowman(resourceDirectory);
}

void Snowman::move()
{
	positionX += speedX;
	positionZ += speedZ;

	//SIZE / 2 is to offset the centering of the terrain
	float terrainX = positionX + Terrain::SIZE / 2;
	float terrainZ = positionZ + Terrain::SIZE / 2;


	float gridSquareSize = Terrain::SIZE / float((Terrain::NUM_VERT - 1));
	int gridX = (int)(terrainX / gridSquareSize);
	int gridZ = (int)(terrainZ / gridSquareSize);

	if (gridX >= Terrain::NUM_VERT - 1 || gridX < 0)
	{
		speedX *= -1;
	}

	if (gridZ >= Terrain::NUM_VERT - 1 || gridZ < 0)
	{
		speedZ *= -1;
	}
	
}

void Snowman::draw()
{
	//////////////////////////////////////////////////////////////////////////
	//Gravity
	playerPositionY += fallSpeed;
	float terrainHeight = terrain->getHeight(positionX, positionZ);

	if (playerPositionY < terrainHeight)
	{
		playerPositionY = terrainHeight;
		fallSpeed = 0.0f;
	}
	if (playerPositionY > terrainHeight)
	{
		fallSpeed -= gravity;
		fallSpeed = min(fallSpeed, 0.1f);
	}
	//////////////////////////////////////////////////////////////////////////

	auto MV = std::make_shared<MatrixStack>();

	// arm rotation by time
	arm_rotate = sin(glfwGetTime());

	// Global
	MV->pushMatrix();
	MV->loadIdentity();
	MV->translate(vec3(positionX, playerPositionY + 5.25f, positionZ));
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

void Snowman::setMaterial(int i) 
{
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

// create a cylinder divided into triangles
void Snowman::createVBO(GLfloat* array)
{
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

void Snowman::createIBO(GLuint* array)
{
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

void Snowman::initSnowman(const std::string& resourceDirectory)
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