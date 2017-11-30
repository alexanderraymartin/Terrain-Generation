#pragma once

#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED

#include "GLSL.h"
#include "glm/glm.hpp"

class Terrain
{
	public:
		Terrain(int _size);
		~Terrain();
		void setHeight(int x, int z, float height);
		float getHeight(int x, int z);
		void computeNormals();
		glm::vec3 getNormal(int x, int z);
		void generateTerrain();
		void createVBO(GLfloat* array);
		void createIBO(GLuint* array);
		void createNormalBuffer(GLfloat * array);
		void renderTerrain();
		float Terrain::randFloat(float l, float h);

		const static int NUM_VERT = 50;
		int size;
		float** heights;
		glm::vec3** normals;
		bool needNormals;
		
		GLuint quad_VertexArrayID;
		GLuint quad_VertexBufferID;
		GLuint quad_IndexBufferID;

		GLfloat g_quad_vertex_buffer_data[(NUM_VERT * NUM_VERT) * 3];
		GLuint g_quad_index_buffer_data[(NUM_VERT - 1) * (NUM_VERT - 1) * 2 * 3];
		GLfloat g_quad_normal_buffer_data[(NUM_VERT * NUM_VERT) * 3];

		//ground plane info
		GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
};

#endif