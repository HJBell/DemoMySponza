#version 330

uniform sampler2DRect sampler_world_position;
uniform sampler2DRect sampler_world_normal;

uniform vec3 light_direction;
uniform float light_intensity = 0.15;

out vec3 reflected_light;

void main(void)
{
	vec3 normal = texture(sampler_world_normal, gl_FragCoord.xy).xyz * 2.0 - 1.0;
	normal = normalize(normal);

	vec3 pos = texture(sampler_world_position, gl_FragCoord.xy).xyz;

	vec3 colour = vec3(0.0);
	colour += max(0.0, dot(light_direction, normal));
	colour *= light_intensity;
	reflected_light = clamp(colour, 0.0, 1.0);
}
