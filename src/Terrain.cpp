#include <iostream>
#include "glm/glm.hpp"
#include "Terrain.h"

using namespace std;
using namespace glm;

Terrain::Terrain(int _size)
{
	width = _size;
	length = _size;

	heights = new float*[length];

	for (int i = 0; i < length; i++)
	{
		heights[i] = new float[width];
	}
	normals = new vec3*[length];

	for (int i = 0; i < length; i++)
	{
		normals[i] = new vec3[width];
	}

	needNormals = true;
}

Terrain::~Terrain()
{
	for (int i = 0; i < length; i++)
	{
		delete[] heights[i];
	}
	delete[] heights;

	for (int i = 0; i < length; i++)
	{
		delete[] normals[i];
	}
	delete[] normals;
}

void Terrain::setHeight(int x, int z, float height)
{
	heights[z][x] = height;
	needNormals = true;
}

float Terrain::getHeight(int x, int z)
{
	return heights[z][x];
}

void Terrain::computeNormals()
{
	needNormals = false;
}

glm::vec3 Terrain::getNormal(int x, int z)
{
	if (needNormals)
	{
		computeNormals();
	}
	return normals[z][x];
}


void Terrain::generateTerrain()
{
	const int NUM_OF_TRIANGLES = 50;
	const int NUM_OF_LEVELS = 5;
	//generate the VAO
	glGenVertexArrays(1, &quad_VertexArrayID);
	glBindVertexArray(quad_VertexArrayID);

	//generate vertex buffer to hand off to OGL
	glGenBuffers(1, &quad_VertexBufferID);

	//set the current state to focus on our vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, quad_VertexBufferID);

	GLfloat g_quad_vertex_buffer_data[(NUM_OF_TRIANGLES * NUM_OF_LEVELS) * 3];
	createVBO(g_quad_vertex_buffer_data);

	GLuint g_quad_index_buffer_data[NUM_OF_TRIANGLES * NUM_OF_LEVELS * 2 * 3];
	createIBO(g_quad_index_buffer_data);

	//actually memcopy the data - only do this once
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_DYNAMIC_DRAW);

	//we need to set up the vertex array
	glEnableVertexAttribArray(0);
	//key function to get up how many elements to pull out at a time (3)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Create and bind IBO
	glGenBuffers(1, &quad_IndexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_IndexBufferID);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_quad_index_buffer_data), g_quad_index_buffer_data, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
}

void Terrain::createVBO(GLfloat * array)
{
	const int NUM_OF_TRIANGLES = 50;
	const int NUM_OF_LEVELS = 5;

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

void Terrain::createIBO(GLuint * array)
{
	const int NUM_OF_TRIANGLES = 50;
	const int NUM_OF_LEVELS = 5;
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
