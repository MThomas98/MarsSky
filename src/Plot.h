#pragma once

#include <glm/glm.hpp>

#include <stdio.h>
#include <vector>

#include "Shader.h"

class Plot {
public:
	Plot(vector<glm::vec3> points);

	void draw(Shader shader);

private:
	vector<glm::vec3> points;
};
