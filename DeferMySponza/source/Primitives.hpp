#pragma once

#include <glm/glm.hpp>
#include <vector>

std::vector<glm::vec3> CubeVerts =
{
	glm::vec3(10.0f, -10.0f, -10.0f), glm::vec3(10.0f, -10.0f,  10.0f), glm::vec3(10.0f,  10.0f,  10.0f), glm::vec3(10.0f,  10.0f,  10.0f), glm::vec3(10.0f,  10.0f, -10.0f), glm::vec3(10.0f, -10.0f, -10.0f),
	glm::vec3(-10.0f, -10.0f,  10.0f), glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec3(-10.0f,  10.0f, -10.0f), glm::vec3(-10.0f,  10.0f, -10.0f), glm::vec3(-10.0f,  10.0f,  10.0f), glm::vec3(-10.0f, -10.0f,  10.0f),
	glm::vec3(-10.0f,  10.0f, -10.0f), glm::vec3(10.0f,  10.0f, -10.0f), glm::vec3(10.0f,  10.0f,  10.0f), glm::vec3(10.0f,  10.0f,  10.0f), glm::vec3(-10.0f,  10.0f,  10.0f), glm::vec3(-10.0f,  10.0f, -10.0f),
	glm::vec3(-10.0f, -10.0f,  10.0f), glm::vec3(10.0f, -10.0f,  10.0f), glm::vec3(10.0f, -10.0f, -10.0f), glm::vec3(10.0f, -10.0f, -10.0f), glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec3(-10.0f, -10.0f,  10.0f),
	glm::vec3(10.0f, -10.0f,  10.0f), glm::vec3(-10.0f, -10.0f,  10.0f), glm::vec3(-10.0f,  10.0f,  10.0f), glm::vec3(-10.0f,  10.0f,  10.0f), glm::vec3(10.0f,  10.0f,  10.0f), glm::vec3(10.0f, -10.0f,  10.0f),
	glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec3(10.0f, -10.0f, -10.0f), glm::vec3(10.0f,  10.0f, -10.0f), glm::vec3(10.0f,  10.0f, -10.0f), glm::vec3(-10.0f,  10.0f, -10.0f), glm::vec3(-10.0f, -10.0f, -10.0f)
};