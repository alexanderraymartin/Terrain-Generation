#pragma once

#ifndef SNOWMAN_H_INCLUDED
#define SNOWMAN_H_INCLUDED

#include "GLSL.h"
#include "glm/glm.hpp"
#include "Terrain.h"

// number of triangles
const int NUM_OF_TRIANGLES = 50;
// number of levels
const int NUM_OF_LEVELS = 5;

class Snowman
{
public:
	Snowman(float _playerPositionX, float _playerPositionY, float _playerPositionZ, int _color, 
		Terrain * _terrain, std::shared_ptr<Program> _prog, std::shared_ptr<Shape> _sphere, 
		GLuint _VertexArrayID, GLuint _VertexBufferID, GLuint _IndexBufferID, const std::string& resourceDirectory);
	void move();
	void draw();
	void setMaterial(int i);

	void createVBO(GLfloat* array);
	void createIBO(GLuint* array);
	void initSnowman(const std::string& resourceDirectory);

	Terrain * terrain = nullptr;
	GLuint VertexArrayID;
	GLuint VertexBufferID;
	GLuint IndexBufferID;

	std::shared_ptr<Program> prog;
	std::shared_ptr<Shape> sphere;
	float positionX = 0.0f;
	float playerPositionY = 0.0f;
	float positionZ = 0.0f;
	int color;

	float speedX;
	float speedZ;
	float arm_rotate = 45.0f;
	float fallSpeed = 0.0f;
	float gravity = 0.02f;
};

#endif
