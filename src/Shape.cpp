#include "Shape.h"
#include <iostream>

#include "GLSL.h"
#include "Program.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "glm/glm.hpp"

using namespace std;
using namespace glm;

void Shape::loadMesh(const string &meshName)
{
	// Load geometry
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> objMaterials;
	string errStr;
	bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());

	if (!rc)
	{
		cerr << errStr << endl;
	}
	else if (shapes.size())
	{
		posBuf = shapes[0].mesh.positions;
		texBuf = shapes[0].mesh.texcoords;
		eleBuf = shapes[0].mesh.indices;
	}
}

// calculate normals
void Shape::calculateNormals() {

	std::vector<vec3> vertices;
	std::vector<vec3> normalVectors;

	for (size_t i = 0; i < posBuf.size(); i++)
	{
		// set up buffer
		norBuf.push_back(0);
	}

	for (size_t i = 0; i < posBuf.size() / 3; i++)
	{
		// set up buffer
		normalVectors.push_back(vec3());
	}

	for (size_t i = 0; i < posBuf.size() / 3; i++)
	{
		// group x, y, z floats into vectors
		float x = posBuf[3 * i + 0];
		float y = posBuf[3 * i + 1];
		float z = posBuf[3 * i + 2];
		vertices.push_back(vec3(x, y, z));
	}

	for (size_t i = 0; i < eleBuf.size(); i += 3)
	{
		// get each point of a triangle
		vec3 a = vertices.at(eleBuf[i + 0]);
		vec3 b = vertices.at(eleBuf[i + 1]);
		vec3 c = vertices.at(eleBuf[i + 2]);

		// get normal
		vec3 normal = normalize(cross(b - a, c - a));

		// b - a, c - a	
		float weight1 = acos(dot(normalize(b - a), normalize(c - a)));
		normalVectors[eleBuf[i + 0]] += weight1 * normal;

		// c - b, a - b
		float weight2 = acos(dot(normalize(c - b), normalize(a - b)));
		normalVectors[eleBuf[i + 1]] += weight2 * normal;

		// a - c, b - c
		float weight3 = acos(dot(normalize(a - c), normalize(b - c)));
		normalVectors[eleBuf[i + 2]] += weight3 * normal;
	}

	for (size_t i = 0; i < norBuf.size() / 3; i++)
	{
		// separate vector into x, y, z floats
		norBuf[i * 3 + 0] = normalVectors[i].x;
		norBuf[i * 3 + 1] = normalVectors[i].y;
		norBuf[i * 3 + 2] = normalVectors[i].z;
	}
}

void Shape::resize()
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	float scaleX, scaleY, scaleZ;
	float shiftX, shiftY, shiftZ;
	float epsilon = 0.001f;

	minX = minY = minZ = 1.1754E+38F;
	maxX = maxY = maxZ = -1.1754E+38F;

	// Go through all vertices to determine min and max of each dimension
	for (size_t v = 0; v < posBuf.size() / 3; v++)
	{
		if (posBuf[3 * v + 0] < minX) minX = posBuf[3 * v + 0];
		if (posBuf[3 * v + 0] > maxX) maxX = posBuf[3 * v + 0];

		if (posBuf[3 * v + 1] < minY) minY = posBuf[3 * v + 1];
		if (posBuf[3 * v + 1] > maxY) maxY = posBuf[3 * v + 1];

		if (posBuf[3 * v + 2] < minZ) minZ = posBuf[3 * v + 2];
		if (posBuf[3 * v + 2] > maxZ) maxZ = posBuf[3 * v + 2];
	}

	// From min and max compute necessary scale and shift for each dimension
	float maxExtent, xExtent, yExtent, zExtent;
	xExtent = maxX - minX;
	yExtent = maxY - minY;
	zExtent = maxZ - minZ;
	if (xExtent >= yExtent && xExtent >= zExtent)
	{
		maxExtent = xExtent;
	}
	if (yExtent >= xExtent && yExtent >= zExtent)
	{
		maxExtent = yExtent;
	}
	if (zExtent >= xExtent && zExtent >= yExtent)
	{
		maxExtent = zExtent;
	}
	scaleX = 2.0f / maxExtent;
	shiftX = minX + (xExtent / 2.0f);
	scaleY = 2.0f / maxExtent;
	shiftY = minY + (yExtent / 2.0f);
	scaleZ = 2.0f / maxExtent;
	shiftZ = minZ + (zExtent / 2.0f);

	// Go through all verticies shift and scale them
	for (size_t v = 0; v < posBuf.size() / 3; v++)
	{
		posBuf[3 * v + 0] = (posBuf[3 * v + 0] - shiftX) * scaleX;
		assert(posBuf[3 * v + 0] >= -1.0f - epsilon);
		assert(posBuf[3 * v + 0] <= 1.0f + epsilon);
		posBuf[3 * v + 1] = (posBuf[3 * v + 1] - shiftY) * scaleY;
		assert(posBuf[3 * v + 1] >= -1.0f - epsilon);
		assert(posBuf[3 * v + 1] <= 1.0f + epsilon);
		posBuf[3 * v + 2] = (posBuf[3 * v + 2] - shiftZ) * scaleZ;
		assert(posBuf[3 * v + 2] >= -1.0f - epsilon);
		assert(posBuf[3 * v + 2] <= 1.0f + epsilon);
	}
}

void Shape::init()
{
	// Initialize the vertex array object
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), posBuf.data(), GL_STATIC_DRAW);

	// Send the normal array to the GPU
	calculateNormals();
	
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), norBuf.data(), GL_STATIC_DRAW);

	// Send the texture array to the GPU
	if (texBuf.empty())
	{
		texBufID = 0;
	}
	else
	{
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size() * sizeof(float), texBuf.data(), GL_STATIC_DRAW);
	}

	// Send the element array to the GPU
	glGenBuffers(1, &eleBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size() * sizeof(unsigned int), eleBuf.data(), GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	assert(glGetError() == GL_NO_ERROR);
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

	glBindVertexArray(vaoID);
	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if (h_nor != -1 && norBufID != 0)
	{
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (texBufID != 0)
	{
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");
		if (h_tex != -1 && texBufID != 0)
		{
			GLSL::enableVertexAttribArray(h_tex);
			glBindBuffer(GL_ARRAY_BUFFER, texBufID);
			glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
		}
	}

	// Bind element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);

	// Draw
	glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);

	// Disable and unbind
	if (h_tex != -1)
	{
		GLSL::disableVertexAttribArray(h_tex);
	}
	if (h_nor != -1)
	{
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
