#include "Plot.h"

Plot::Plot(vector<glm::vec3> points) {
	this->points = points;
}

void Plot::draw(Shader shader) {
	shader.use();
}
