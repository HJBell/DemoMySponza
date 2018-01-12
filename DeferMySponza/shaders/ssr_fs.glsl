#version 330

uniform sampler2DRect cpp_PositionTex;
uniform sampler2DRect cpp_NormalTex;
uniform sampler2DRect cpp_ColourTex;
uniform sampler2DRect cpp_MaterialTex;

uniform vec2 cpp_WindowDims;
uniform vec3 cpp_CameraPos;
uniform mat4 cpp_ViewProjectionMatrix;

out vec3 fs_Colour;

vec2 WorldPosToScreenCoords(vec3 worldPos) 
{
	// Converting the world space position to a screen space coordinate.
	vec4 point3D = (cpp_ViewProjectionMatrix * vec4(worldPos, 1.0));
	if (point3D.w == 0.0)
		return vec2(0.0);
	vec3 ndcPos = point3D.xyz / point3D.w;
	return ((ndcPos.xy + 1.0) / 2.0) * cpp_WindowDims;
}

void main(void)
{
	// Getting the fragment data from the gbuffer textures.
	vec3 pos = texture(cpp_PositionTex, gl_FragCoord.xy).xyz;
	vec3 normal = normalize(texture(cpp_NormalTex, gl_FragCoord.xy).xyz);
	vec3 fragCol = texture(cpp_ColourTex, gl_FragCoord.xy).rgb;
	vec3 matInfo = texture(cpp_MaterialTex, gl_FragCoord.xy).rgb;
	
	vec3 colour = vec3(0.0);

	// Checking if the current fragment has SSR applied.
	if (matInfo.r == 1.0)
	{
		// Calculating the reflected vector.
		vec3 fragmentToCamera = cpp_CameraPos - pos;
		vec3 reflectionVector = reflect(-fragmentToCamera, normal);
		vec3 refVecNormalised = normalize(reflectionVector);

		// Stepping the ray forward along the reflected vector.
		float stepDistance = 1.0;
		for (int i = 1; i < 100; i++)
		{
			vec3 rayPos = pos + refVecNormalised * i * stepDistance;
			vec2 rayScreenCoords = WorldPosToScreenCoords(rayPos);

			// Exiting if the ray leaves the screen.
			if (rayScreenCoords.x < 0.0 || rayScreenCoords.y < 0.0)
				break;
			if (rayScreenCoords.x > cpp_WindowDims.x || rayScreenCoords.y > cpp_WindowDims.y)
				break;

			// Getting the position of the fragment the ray is on.
			vec3 rayFragPos = texture(cpp_PositionTex, rayScreenCoords.xy).xyz;

			// Checking if the ray is behind the fragment and close to it.
			float l1 = length(rayPos - cpp_CameraPos);
			float l2 = length(rayFragPos - cpp_CameraPos);
			if (l1 >= l2 && abs(l1 - l2) <= stepDistance * 2)
			{
				// Getting the colour of the fragment as the reflected colour.
				colour = texture(cpp_ColourTex, rayScreenCoords.xy).rgb;
				break;
			}
		}
	}	

	// Outputting the colour.
	fs_Colour = clamp(colour * 0.2, 0.0, 1.0);
}
