#include "Decal.h"

static const float PLANE_VERTS[] = {
	 1.0f,  0.0f,  1.0f,  1.0f, 1.0f,
	-1.0f,  0.0f,  1.0f,  0.0f, 1.0f,
	-1.0f,  0.0f, -1.0f,  0.0f, 0.0f,

	 1.0f,  0.0f,  1.0f,  1.0f, 1.0f,
	-1.0f,  0.0f, -1.0f,  0.0f, 0.0f,
	 1.0f,  0.0f, -1.0f,  1.0f, 0.0f
};

Decal::Decal(const char* path) {
	// Load in the texture data
	int width, height, numComponents;
	unsigned char* data = stbi_load(path, &width, &height, &numComponents, 0);
	if (!data) {
		printf("ERROR: Failed to load decal texture %s.\n", path);
		return;
	}

	// Determine the format of the data
	GLenum format;
	if (numComponents == 1)
		format = GL_RED;
	if (numComponents == 3)
		format = GL_RGB;
	if (numComponents == 4)
		format = GL_RGBA;

	// Generate the OpenGL texture
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Free the image data (now stored in OpenGL texture)
	stbi_image_free(data);

	// Generate the mesh objects
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	// Bind the mesh objects
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Buffer the data
	glBufferData(GL_ARRAY_BUFFER, sizeof(PLANE_VERTS), &PLANE_VERTS, GL_STATIC_DRAW);

	// Setup vao data positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// Unbind vao
	glBindVertexArray(0);
}

void Decal::draw(Shader shader) {
	// Activate texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	shader.setInt("tex", textureID);

	// Bind the vao and draw
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Unbind everything
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
