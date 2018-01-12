#version 330

uniform samplerCube cpp_CubeMap;

in vec3 vs_TextureCoord;

out vec4 fs_Colour;

void main()
{
	// Outputting the colour of the skybox for this fragment based on the texture cube.
	fs_Colour = texture(cpp_CubeMap, vs_TextureCoord);
}