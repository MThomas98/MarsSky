#include "Mesh.h"

void Mesh::setupMesh() {
	// Generate the mesh objects
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	// Bind the mesh objects
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	// Buffer the data
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Setup vao data (0 = position, 1 = normal, 2 = texture coords)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, norm));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

	// Unbind vao
	glBindVertexArray(0);
}

void Mesh::draw(Shader shader) {
	// Search for the first diffuse and spec texture and assign them texture units
	for (unsigned int i = 0; i < textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		bool foundDiff = false;
		bool foundSpec = false;

		shader.setInt("material.shininess", 1);

		if (!foundDiff && textures[i].type == "diffuse") {
			foundDiff = true;
			shader.setInt("material.diffuse", i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		if (!foundSpec && textures[i].type == "specular") {
			foundSpec = true;
			shader.setInt("material.specular", i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
	}

	// Bind and draw
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	// Unbind everying
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}
