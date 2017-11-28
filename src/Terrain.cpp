#include <iostream>
#include "glm/glm.hpp"
#include "Terrain.h"

using namespace std;
using namespace glm;

Terrain::Terrain(int _size)
{
	size = _size;

	heights = new float*[size];

	for (int i = 0; i < size; i++)
	{
		heights[i] = new float[size];
	}
	normals = new vec3*[size];

	for (int i = 0; i < size; i++)
	{
		normals[i] = new vec3[size];
	}

	needNormals = true;
}

Terrain::~Terrain()
{
	for (int i = 0; i < size; i++)
	{
		delete[] heights[i];
	}
	delete[] heights;

	for (int i = 0; i < size; i++)
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
	//generate the VAO
	glGenVertexArrays(1, &quad_VertexArrayID);
	glBindVertexArray(quad_VertexArrayID);

	//generate vertex buffer to hand off to OGL
	glGenBuffers(1, &quad_VertexBufferID);

	//set the current state to focus on our vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, quad_VertexBufferID);

	GLfloat g_quad_vertex_buffer_data[(NUM_VERT * NUM_VERT) * 3];
	createVBO(g_quad_vertex_buffer_data);

	GLuint g_quad_index_buffer_data[(NUM_VERT - 1) * (NUM_VERT - 1) * 2 * 3];
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
	// new point
	float newX = 0.0f;
	float newZ = 0.0f;

	for (int y = 0; y < NUM_VERT; y++) {
		for (int x = 0; x < NUM_VERT; x++) {
			array[3 * x + NUM_VERT * y * 3] = newX; // x coordinate
			array[3 * x + 1 + NUM_VERT * y * 3] = 0; // y coordinate
			array[3 * x + 2 + NUM_VERT * y * 3] = newZ; // z coordinate

			newX += size / (float)NUM_VERT;
		}
		newX = 0.0f;
		newZ += size / (float)NUM_VERT;
	}
}

void Terrain::createIBO(GLuint * array)
{
	for (int y = 0; y < NUM_VERT - 1; y++)
	{
		for (int x = 0; x < NUM_VERT - 1; x++)
		{
			// Triangles Down
			int index = (x + y * (NUM_VERT - 1)) * 6;
			int k = x + (y * NUM_VERT);

			array[index + 0] = k;
			array[index + 1] = k + 1;
			array[index + 2] = k + NUM_VERT;

			// Triangles Up
			array[index + 3] = k + 1;
			array[index + 4] = k + NUM_VERT;
			array[index + 5] = k + NUM_VERT + 1;
		}
	}
}
