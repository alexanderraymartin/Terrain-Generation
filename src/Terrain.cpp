#include <iostream>
#include "glm/glm.hpp"
#include "Terrain.h"

using namespace std;
using namespace glm;

Terrain::Terrain()
{
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

	isWireFrame = true;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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

// uses barry centrix coordinates to find the height
// algorithm adapted from ThinMatrix on YouTube 
float Terrain::getHeight(float worldX, float worldZ)
{
	//SIZE / 2 is to offset the centering of the terrain
	float terrainX = worldX + SIZE / 2;
	float terrainZ = worldZ + SIZE / 2;


	float gridSquareSize = SIZE / float((NUM_VERT - 1));
	int gridX = (int)(terrainX / gridSquareSize);
	int gridZ = (int)(terrainZ / gridSquareSize);

	if (gridX >= NUM_VERT - 1 || gridZ >= NUM_VERT - 1 || gridX < 0 || gridZ < 0)
	{
		return 0;
	}

	float xCoord = (fmod(terrainX, gridSquareSize)) / gridSquareSize;
	float zCoord = (fmod(terrainZ, gridSquareSize)) / gridSquareSize;
	float answer;

	if (xCoord <= (1 - zCoord))
	{
		answer = barryCentric(vec3(0, heights[gridZ][gridX], 0), vec3(1, heights[gridZ + 1][gridX], 0), vec3(0, heights[gridZ][gridX + 1], 1), vec2(xCoord, zCoord));
	}
	else
	{
		answer = barryCentric(vec3(1, heights[gridZ + 1][gridX], 0), vec3(1, heights[gridZ + 1][gridX + 1], 1), vec3(0, heights[gridZ][gridX + 1], 1), vec2(xCoord, zCoord));
	}

	return answer;

}

float Terrain::computeHeight(int x, int z)
{
	float height = 0;
	float d = pow(2, OCTAVES - 1);
	for (int i = 0; i < OCTAVES; i++)
	{
		float freq = pow(2, i) / d;
		float amplitude = pow(ROUGHNESS, i) * AMPLITUDE;
		height += getInterpolatedNoise(x * freq, z * freq) * amplitude;
	}
	return height;
}

float Terrain::randFloat(float l, float h)
{
	float r = rand() / (float)RAND_MAX;
	return (1.0f - r) * l + r * h;
}

void Terrain::computeNormals()
{
	for (int z = 0; z < NUM_VERT; z++)
	{
		for (int x = 0; x < NUM_VERT; x++)
		{
			float heightL = computeHeight(x - 1, z);
			float heightR = computeHeight(x + 1, z);
			float heightD = computeHeight(x, z - 1);
			float heightU = computeHeight(x, z + 1);
			vec3 normal = vec3(heightL - heightR, 2.0f, heightD - heightU);
			normals[z][x] = normalize(normal);
		}
	}
}

glm::vec3 Terrain::getNormal(int x, int z)
{
	if (needNormals && !isWireFrame)
	{
		needNormals = false;
		computeNormals();
	}
	if (isWireFrame)
	{
		return vec3(0.0f, 1.0f, 0.0f);
	}
	return normals[z][x];
}

void Terrain::getNewTerrain()
{
	needNormals = true;
	isWireFrame = true;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	generateTerrain();
}

void Terrain::renderSolidTerrain()
{
	isWireFrame = false;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	createNormalBuffer(g_quad_normal_buffer_data);
	glGenBuffers(1, &GrndNorBuffObj);
	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_normal_buffer_data), g_quad_normal_buffer_data, GL_STATIC_DRAW);

}

void Terrain::generateTerrain()
{
	seed = rand() % 1000000000;
	createVBO(g_quad_vertex_buffer_data);
	createIBO(g_quad_index_buffer_data);
	createNormalBuffer(g_quad_normal_buffer_data);

	int size = 1000;
	float GrndTex[] = {
		0, 0, // back
		0, size,
		size, size,
		size, 0
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

	for (int z = 0; z < NUM_VERT; z++)
	{
		for (int x = 0; x < NUM_VERT; x++)
		{
			float height = computeHeight(newX, newZ);
			array[3 * x + NUM_VERT * z * 3] = newX; // x coordinate
			array[3 * x + 1 + NUM_VERT * z * 3] = height; // y coordinate
			heights[z][x] = height;
			array[3 * x + 2 + NUM_VERT * z * 3] = newZ; // z coordinate
			newX += SIZE / (float)NUM_VERT;
		}
		newX = 0.0f;
		newZ += SIZE / (float)NUM_VERT;
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
	glDrawElements(GL_TRIANGLES, (NUM_VERT - 1) * (NUM_VERT - 1) * 6, GL_UNSIGNED_INT, nullptr);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

// get the noise
float Terrain::getNoise(int x, int z)
{
	srand(x * 48512 + z * 324935 + seed);
	return randFloat(0, 1) * 2.0f - 1.0f;
}

// get the smooth noise
float Terrain::getSmoothNoise(int x, int z)
{
	float corners = (getNoise(x - 1, z - 1) + getNoise(x + 1, z - 1) + getNoise(x - 1, z + 1) + getNoise(x + 1, z + 1)) / 16.0f;
	float sides = (getNoise(x - 1, z) + getNoise(x + 1, z) + getNoise(x, z - 1) + getNoise(x, z + 1)) / 8.0f;
	float center = getNoise(x, z) / 4.0f;

	return corners + sides + center;
}

// get the interpolated value given a point
float Terrain::getInterpolatedNoise(float x, float z)
{
	int integerX = (int)x;
	int integerZ = (int)z;
	float fractionX = x - integerX;
	float fractionZ = z - integerZ;

	float vertex1 = getSmoothNoise(integerX, integerZ);
	float vertex2 = getSmoothNoise(integerX + 1, integerZ);
	float vertex3 = getSmoothNoise(integerX, integerZ + 1);
	float vertex4 = getSmoothNoise(integerX + 1, integerZ + 1);

	float interpolate1 = interpolate(vertex1, vertex2, fractionX);
	float interpolate2 = interpolate(vertex3, vertex4, fractionX);

	return interpolate(interpolate1, interpolate2, fractionZ);
}

// interpolate two values using cosine interpolation with a specified blend amount
float Terrain::interpolate(float a, float b, float blend)
{
	float theta = blend * 3.14159265359f;
	float f = (1.0f - cos(theta)) * 0.5f;
	return a * (1.0f - f) + b * f;
}

// standard barry centric algorithm
float Terrain::barryCentric(vec3 p1, vec3 p2, vec3 p3, vec2 pos)
{
	float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
	float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
	float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
	float l3 = 1.0f - l1 - l2;
	return l1 * p1.y + l2 * p2.y + l3 * p3.y;
}