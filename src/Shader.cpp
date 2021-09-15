#include "Shader.h"

Shader::Shader(const char* vPath, const char* fPath) {
	// Variables where cstrs will be stored
	string vStr;
	string fStr;
	const char* vCode;
	const char* fCode;

	// Setup file readers
	ifstream vFile;
	ifstream fFile;
	vFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	fFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

	try {
		// Open files
		vFile.open(vPath);
		fFile.open(fPath);

		// Read in from files to string
		vStr = string((istreambuf_iterator<char>(vFile)), istreambuf_iterator<char>());
		fStr = string((istreambuf_iterator<char>(fFile)), istreambuf_iterator<char>());

		// Close files
		vFile.close();
		fFile.close();

		// Store cstrs for use later
		vCode = vStr.c_str();
		fCode = fStr.c_str();
	} catch(ifstream::failure e) {
		printf("ERROR: Could not read shader file.\n");
	}

	// Compile the vertex shader
	unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, &vCode, NULL);
	glCompileShader(vShader);

	// Compile the vertex shader
	unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, &fCode, NULL);
	glCompileShader(fShader);

	// Check if shaders compiled correctly
	int success;
	char infoLog[512];
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vShader, 512, NULL, infoLog);
		printf("ERROR: Vertex shader compilation failed.\n%s\n%s\n", infoLog, vCode);
	}
	glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fShader, 512, NULL, infoLog);
		printf("ERROR: Fragment shader compilation failed.\n%s\n%s\n", infoLog, fCode);
	}

	// Create a shader program from the compiled shaders
	shaderProg = glCreateProgram();
	glAttachShader(shaderProg, vShader);
	glAttachShader(shaderProg, fShader);
	glLinkProgram(shaderProg);

	// Check if the program linked correctly
	glGetProgramiv(shaderProg, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(shaderProg, 512, NULL, infoLog);
		printf("ERROR: Failed to link shader program.\n%s\n", infoLog);
	}

	// Cleanup the leftover free shaders as they are now in the program
	glDeleteShader(vShader);
	glDeleteShader(fShader);

}

void Shader::use() {
	glUseProgram(shaderProg);
}

void Shader::setBool(string name, bool value) const { glUniform1i(glGetUniformLocation(shaderProg, name.c_str()), (int)value); }
void Shader::setInt(string name, int value) const { glUniform1i(glGetUniformLocation(shaderProg, name.c_str()), value); }
void Shader::setFloat(string name, float value) const { glUniform1f(glGetUniformLocation(shaderProg, name.c_str()), value); }
void Shader::setVec4f(string name, float v1, float v2, float v3, float v4) const { glUniform4f(glGetUniformLocation(shaderProg, name.c_str()), v1, v2, v3, v4); }
void Shader::setVec3f(string name, float v1, float v2, float v3) const { glUniform3f(glGetUniformLocation(shaderProg, name.c_str()), v1, v2, v3); }
void Shader::setMat4f(string name, glm::mat4 value) const { glUniformMatrix4fv(glGetUniformLocation(shaderProg, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); }
