#version 330

uniform sampler2DRect cpp_ColourTex;

out vec3 fs_Colour;

mat3 sx = mat3(
	1.0, 2.0, 1.0,
	0.0, 0.0, 0.0,
	-1.0, -2.0, -1.0
);
mat3 sy = mat3(
	1.0, 0.0, -1.0,
	2.0, 0.0, -2.0,
	1.0, 0.0, -1.0
);

//------------
mat3 kernal = mat3(
	1.0, 2.0, 1.0,
	2.0, 4.0, 2.0,
	1.0, 2.0, 1.0
);
//------------


void main()
{
	vec3 colour = texture(cpp_ColourTex, gl_FragCoord.xy).rgb;
	mat3 I;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			vec3 sample = texture(cpp_ColourTex, ivec2(gl_FragCoord.xy) + ivec2(i - 1, j - 1)).rgb;
			I[i][j] = length(sample);
		}
	}

	float gx = dot(sx[0], I[0]) + dot(sx[1], I[1]) + dot(sx[2], I[2]);
	float gy = dot(sy[0], I[0]) + dot(sy[1], I[1]) + dot(sy[2], I[2]);

	float g = sqrt(pow(gx, 2.0) + pow(gy, 2.0));

	if (g > 0.95)
	{
		vec3 blurredColour = vec3(0.0);
		for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			blurredColour += texture(cpp_ColourTex, ivec2(gl_FragCoord.xy) + ivec2(i - 1, j - 1)).rgb * kernal[i][j];
		}
		}
		blurredColour /= 16.0;
		colour = blurredColour;

		//colour = vec3(1.0, 0.0, 1.0);
	}

	fs_Colour = colour;	
}
