#version 330

uniform sampler2DRect cpp_PositionTex;
uniform sampler2DRect cpp_NormalTex;
uniform sampler2DRect cpp_ColourTex;

uniform vec3 cpp_LightPos;
uniform vec3 cpp_LightIntensity;
uniform float cpp_LightRange;

out vec3 fs_Colour;

void main(void)
{
	// Getting the fragment data from the gbuffer textures.
	vec3 pos = texture(cpp_PositionTex, gl_FragCoord.xy).xyz;
	vec3 normal = normalize(texture(cpp_NormalTex, gl_FragCoord.xy).xyz);
	vec3 fragCol = texture(cpp_ColourTex, gl_FragCoord.xy).rgb;

	vec3 colour = vec3(0.0);

	// Checking if the fragment is within the range of the light.
	vec3 fragmentToLight = cpp_LightPos - pos;
	float distanceToLight = length(fragmentToLight);
	float rangeIntensity = (1.0 - smoothstep(0, cpp_LightRange, distanceToLight));
	if (rangeIntensity > 0.0)
	{
		// Calculating the intesity of the light on the fragment due to its angle relactive to the light source.
		float angleIntensity = max(0.0, dot(normalize(fragmentToLight), normal));

		// Combining the various intesity calculations to give the final colour.
		colour = fragCol * angleIntensity * cpp_LightIntensity * rangeIntensity;
	}

	// Outputting the colour.
	fs_Colour = clamp(colour, 0.0, 1.0);
}
