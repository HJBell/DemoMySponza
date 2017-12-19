#version 330

uniform sampler2DRect cpp_NormalTex;
uniform vec3 cpp_LightDir;
uniform vec3 cpp_LightIntensity;

out vec3 fs_Colour;

void main(void)
{
	vec3 normal = texture(cpp_NormalTex, gl_FragCoord.xy).xyz;
	normal = normalize(normal);

	vec3 colour = vec3(0.0);
	colour += max(0.0, dot(cpp_LightDir, normal));
	colour *= cpp_LightIntensity;
	fs_Colour = clamp(colour, 0.0, 1.0);
}
