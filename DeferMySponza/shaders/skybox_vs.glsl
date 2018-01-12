#version 330

layout(std140) uniform cpp_SkyboxUniforms
{
	mat4 cpp_ViewProjectionXform;
	vec3 cpp_CameraPos;
};

in vec3 cpp_VertexPosition;

out vec3 vs_TextureCoord;

void main(void)
{
	vs_TextureCoord = vec3(cpp_VertexPosition.x, -cpp_VertexPosition.yz);
	gl_Position = cpp_ViewProjectionXform * vec4(cpp_VertexPosition + cpp_CameraPos, 1.0);
}

