#pragma once

#include <glm/glm.hpp>

#define MAX_INSTANCE_COUNT 64

struct DirectionalLight
{
	glm::vec3 direction;
	float PADDING1;
	glm::vec3 intensity;
	float PADDING2;
};

struct PointLight
{
	glm::vec3 position;
	float range;
	glm::vec3 intensity;
	float PADDING0;
};

struct SpotLight
{
	glm::vec3 position;
	float range;
	glm::vec3 intensity;
	float angle;
	glm::vec3 direction;
	float PADDING0;
};

struct InstanceData
{
	glm::mat4 mvpXform;
	glm::mat4 modelXform;
	glm::vec3 diffuse;
	float shininess;
	glm::vec3 specular;
	int isShiny;
};

struct PerFrameUniforms
{
	glm::vec3 cameraPos;
	float PADDING0;
	glm::vec3 ambientIntensity;
};

struct PerModelUniforms
{
	InstanceData instances[MAX_INSTANCE_COUNT];
};

struct DirectionalLightUniforms
{
	DirectionalLight light;
};

struct PointLightUniforms
{
	PointLight light;
};

struct SpotLightUniforms
{
	SpotLight light;
};

struct SkyboxUniforms
{
	glm::mat4 viewProjectionXform;
	glm::vec3 cameraPos;
};

