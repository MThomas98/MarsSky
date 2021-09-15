#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <cstdio>
#include <string>
#include <vector>

#include "Shader.h"

using namespace std;

struct Vertex {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 texCoord;
};

struct Texture {
	GLuint id;
	string type;
	string path;
};

class Mesh {
public:
	Mesh(vector<Vertex> verts, vector<unsigned int> indis, vector<Texture> texts) {
		this->vertices = verts;
		this->indices = indis;
		this->textures = texts;
		setupMesh();
	}

	void draw(Shader shader);
private:
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	GLuint vao, vbo, ebo;

	void setupMesh();
};
