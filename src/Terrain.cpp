#include <iostream>
#include "glm/glm.hpp"
#include "Terrain.h"

using namespace std;
using namespace glm;

Terrain::Terrain(int _size)
{
	size = _size;

	heights = new float*[NUM_VERT];

	for (int i = 0; i < NUM_VERT; i++)
	{
		heights[i] = new float[NUM_VERT];
	}
	normals = new vec3*[NUM_VERT];

	for (int i = 0; i < NUM_VERT; i++)
	{
		normals[i] = new vec3[NUM_VERT];
	}

	needNormals = true;
}

Terrain::~Terrain()
{
	for (int i = 0; i < NUM_VERT; i++)
	{
		delete[] heights[i];
	}
	delete[] heights;

	for (int i = 0; i < NUM_VERT; i++)
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
	return randFloat(0.0, 1.0); // [z][x];
}

void Terrain::computeNormals()
{
	needNormals = false;
	for (int z = 0; z < NUM_VERT; z++)
	{
		for (int x = 0; x < NUM_VERT; x++)
		{
			float heightL = getHeight(x - 1, z);
			float heightR = getHeight(x + 1, z);
			float heightD = getHeight(x, z - 1);
			float heightU = getHeight(x, z + 1);
			vec3 normal = vec3(heightL - heightR, 2.0f, heightD - heightU);
			normals[z][x] = normalize(normal);
		}
	}
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
	createVBO(g_quad_vertex_buffer_data);
	createIBO(g_quad_index_buffer_data);
	createNormalBuffer(g_quad_normal_buffer_data);

	float GrndTex[] = {
		0, 0, // back
		0, 20,
		20, 20,
		20, 0
	};

	GLuint VertexArrayID;
	//generate the VAO
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &GrndBuffObj);
	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &GrndNorBuffObj);
	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_normal_buffer_data), g_quad_normal_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &GrndTexBuffObj);
	glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

	glGenBuffers(1, &GIndxBuffObj);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_quad_index_buffer_data), g_quad_index_buffer_data, GL_STATIC_DRAW);
}

void Terrain::createVBO(GLfloat * array)
{
	// new point
	float newX = 0.0f;
	float newZ = 0.0f;

	for (int y = 0; y < NUM_VERT; y++) 
	{
		for (int x = 0; x < NUM_VERT; x++) 
		{
			array[3 * x + NUM_VERT * y * 3] = newX; // x coordinate
			array[3 * x + 1 + NUM_VERT * y * 3] = getHeight(newX, newZ); // y coordinate
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

void Terrain::createNormalBuffer(GLfloat * array)
{

	for (int z = 0; z < NUM_VERT; z++) 
	{
		for (int x = 0; x < NUM_VERT; x++) 
		{
			vec3 normal = getNormal(x, z);
			array[3 * x + NUM_VERT * z * 3] = normal.x; // x coordinate
			array[3 * x + 1 + NUM_VERT * z * 3] = normal.y; // y coordinate
			array[3 * x + 2 + NUM_VERT * z * 3] = normal.z; // z coordinate
		}
	}
}

void Terrain::renderTerrain()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// draw!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
	glDrawElements(GL_TRIANGLES, NUM_VERT * NUM_VERT * 6, GL_UNSIGNED_INT, nullptr);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

float Terrain::randFloat(float l, float h)
{
	float r = rand() / (float)RAND_MAX;
	return (1.0f - r) * l + r * h;
}