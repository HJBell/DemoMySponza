#version 330

uniform sampler2DRect cpp_ColourTex;
uniform vec3 cpp_AmbientIntensity;

out vec3 fs_Colour;

void main(void)
{
	vec3 fragCol = texture(cpp_ColourTex, gl_FragCoord.xy).rgb;

	fs_Colour = clamp(cpp_AmbientIntensity * fragCol, 0.0, 1.0);
}