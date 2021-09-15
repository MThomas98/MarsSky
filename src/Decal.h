#pragma once

#include <GL/glew.h>

#include <stb_image.h>

#include "Shader.h"

class Decal {
public:
	Decal(const char* path);

	void draw(Shader shader);
private:
	GLuint textureID;
	GLuint vao, vbo;
};
