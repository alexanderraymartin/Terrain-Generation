#pragma once

#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED

#include "GLSL.h"
#include "glm/glm.hpp"

class Terrain
{
	public:
		Terrain();
		~Terrain();
		float getHeight(float x, float z);
		float computeHeight(int x, int z);
		float Terrain::randFloat(float l, float h);
		void computeNormals();
		glm::vec3 getNormal(int x, int z);
		void getNewTerrain();

		void renderSolidTerrain();
		void generateTerrain();
		void createVBO(GLfloat* array);
		void createIBO(GLuint* array);
		void createNormalBuffer(GLfloat * array);
		void renderTerrain();
		float getNoise(int x, int z);
		float getInterpolatedNoise(float x, float z);
		float getSmoothNoise(int x, int z);
		float interpolate(float a, float b, float blend);
		float barryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos);

		const static int NUM_VERT = 300;
		const static int SIZE = 500;
		float AMPLITUDE = 200.0f;
		const static int OCTAVES = 7;
		float ROUGHNESS = 0.25f;
		
		// good values ~ 25 second load
		//const static int NUM_VERT = 300;
		//const static int SIZE = 500;
		//float AMPLITUDE = 200.0f;
		//const static int OCTAVES = 7;
		//float ROUGHNESS = 0.25f;


		int seed;
		float** heights;
		glm::vec3** normals;
		bool needNormals;
		bool isWireFrame;

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