#version 330

#define MAX_INSTANCE_COUNT 64

struct InstanceData
{
	mat4 mvpXform;
	mat4 modelXform;
	vec3 diffuse;
	float shininess;
	vec3 specular;
	int isShiny;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 intensity;
};

layout(std140) uniform cpp_PerFrameUniforms
{
	vec3 cpp_CameraPos;
	vec3 cpp_AmbientIntensity;
};

layout(std140) uniform cpp_PerModelUniforms
{
	InstanceData cpp_Instances[MAX_INSTANCE_COUNT];
};

layout(std140) uniform cpp_DirectionalLightUniforms
{
	DirectionalLight cpp_Light;
};

uniform sampler2D cpp_Texture;
uniform int cpp_EnableSSR;

in vec3 vs_Position;
in vec3 vs_Normal;
in vec2 vs_TextureCoord;
flat in int vs_InstanceID;

out vec3 fs_Position;
out vec3 fs_Normal;
out vec3 fs_Colour;
out vec3 fs_Material;

void main(void)
{	
	// Outputting the mesh data to the gbuffer textures.
	fs_Position = vs_Position;
	fs_Normal = vs_Normal;
	fs_Colour = cpp_Instances[vs_InstanceID].diffuse * texture(cpp_Texture, vs_TextureCoord).rgb;
	fs_Material = vec3(cpp_EnableSSR, 0.0, 0.0);
}