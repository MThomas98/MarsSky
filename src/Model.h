#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdio.h>
#include <string>
#include <vector>

#include "Mesh.h"
#include "Shader.h"

class Model {
public:
	Model(const char* path) { loadModel(path); }

	void draw(Shader shader);

private:
	vector<Mesh> meshes;
	vector<Texture> loadedTextures;
	string directory;

	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
	unsigned int textureFromFile(const char* path, const string &directory, bool gamma = false);
};
