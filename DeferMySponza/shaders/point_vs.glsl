#version 330

uniform mat4 cpp_MVP;

in vec3 cpp_Position;

void main(void)
{
	gl_Position = cpp_MVP * vec4(cpp_Position, 1.0);
}
