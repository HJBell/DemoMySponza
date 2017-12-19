#version 330

uniform sampler2DRect cpp_PositionTex;
uniform sampler2DRect cpp_NormalTex;
uniform sampler2DRect cpp_ColourTex;

uniform vec3 cpp_LightPos;
uniform vec3 cpp_LightIntensity;
uniform float cpp_LightRange;
uniform float cpp_LightAngle;
uniform vec3 cpp_LightDir;

out vec3 fs_Colour;

void main(void)
{
	vec3 pos = texture(cpp_PositionTex, gl_FragCoord.xy).xyz;
	vec3 normal = normalize(texture(cpp_NormalTex, gl_FragCoord.xy).xyz);
	vec3 fragCol = texture(cpp_ColourTex, gl_FragCoord.xy).rgb;

	vec3 colour = vec3(0.0);
	vec3 lightDirection = normalize(cpp_LightDir);
	vec3 lightToFragment = pos - cpp_LightPos;
	float angleBetweenLightAndRay = dot(lightDirection, normalize(lightToFragment));
	if (angleBetweenLightAndRay > cos(cpp_LightAngle))
	{
		float distanceToLight = length(-lightToFragment);
		float rangeIntensity = (1.0 - smoothstep(0, cpp_LightRange, distanceToLight));
		if (rangeIntensity > 0.0)
		{
			float angleIntensity = max(0.0, dot(normalize(-lightToFragment), normal));
			float angularAttenuation = smoothstep(cos(cpp_LightAngle), cos(cpp_LightAngle * 0.9), angleBetweenLightAndRay);
			colour = fragCol * angleIntensity * cpp_LightIntensity * rangeIntensity * angularAttenuation;
		}
	}

	fs_Colour = clamp(colour, 0.0, 1.0);
}
