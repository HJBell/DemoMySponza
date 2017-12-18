#include "ShaderProgram.hpp"
#include <tygra/FileHelper.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>


//-------------------------------------Public Functions-------------------------------------

ShaderProgram::ShaderProgram()
{
}


ShaderProgram::~ShaderProgram()
{
}

void ShaderProgram::Use() const
{
	// Telling OpenGL to use this shader program.
	glUseProgram(mProgramID);
}

void ShaderProgram::Init(std::string vertexShaderPath, std::string fragmentShaderPath)
{
	// Loading and compiling the vertex and fragment shader to be used in the shader program.
	GLuint vertexShaderID = LoadShader(vertexShaderPath, GL_VERTEX_SHADER);
	GLuint fragmentShaderID = LoadShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

	// Creating the shader program.
	mProgramID = glCreateProgram();

	// Attaching the shaders to the shader program.
	glAttachShader(mProgramID, vertexShaderID);
	glAttachShader(mProgramID, fragmentShaderID);

	// Linking the shader program
	glLinkProgram(mProgramID);

	// Check that the shader program linked correctly.
	GLint linkSuccessful = GL_FALSE;
	glGetProgramiv(mProgramID, GL_LINK_STATUS, &linkSuccessful);
	if (linkSuccessful != GL_TRUE)
	{
		// Outputting an error message if the shader program failed to link correctly.
		int infoLogLength = 0;
		glGetProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> infoLog(infoLogLength + 1);
		glGetShaderInfoLog(mProgramID, infoLogLength, NULL, &infoLog[0]);
		std::cerr << "Error linking shader program : " << std::endl << &infoLog[0] << std::endl;
	}
}

void ShaderProgram::CreateUniformBuffer(std::string name, GLsizeiptr size, int index)
{
	// Generating a buffer object and binding it.
	mUniformBuffers[name] = 0;
	glGenBuffers(1, &mUniformBuffers[name]);
	glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffers[name]);

	// Configuring the buffer to be used as a uniform buffer block.
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, index, mUniformBuffers[name]);
	glUniformBlockBinding(mProgramID, glGetUniformBlockIndex(mProgramID, name.c_str()), index);
}

void ShaderProgram::SetUniformBuffer(std::string name, const void * data, GLsizeiptr size) const
{
	// Binding the uniform buffer block.
	glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffers.at(name));

	// Populating the buffer block.
	glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
}

void ShaderProgram::SetTextureUniform(GLuint textureID, std::string uniformName) const
{
	// Binding the current texture.
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Populating the uniform with the texture.
	glUniform1i(glGetUniformLocation(mProgramID, uniformName.c_str()), 0);
}

void ShaderProgram::SetTextureCubeUniform(GLuint textureID, std::string uniformName) const
{
	// Binding the current texture cube.
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	// Populating the uniform with the texture cube.
	glUniform1i(glGetUniformLocation(mProgramID, uniformName.c_str()), 0);
}

void ShaderProgram::Dispose()
{
	// Deleting the shader program if it is not set to 0.
	if(mProgramID != 0)
		glDeleteProgram(mProgramID);
}


//------------------------------------Private Functions-------------------------------------

GLuint ShaderProgram::LoadShader(std::string shaderPath, GLuint shaderType) const
{
	// Create the shader object.
	GLuint shaderID = glCreateShader(shaderType);

	// Read the shader code from its file.
	std::string shaderString = tygra::createStringFromFile(shaderPath);
	auto shaderCString = shaderString.c_str();

	// Compile the shader.
	glShaderSource(shaderID, 1, &shaderCString, NULL);
	glCompileShader(shaderID);

	// Check the shader compiled correctly.
	GLint compileSuccessful = GL_FALSE;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileSuccessful);
	if (compileSuccessful != GL_TRUE)
	{
		// Outputting an error if the shader failed to compile correctly.
		int infoLogLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<char> infoLog(infoLogLength + 1);
		glGetShaderInfoLog(shaderID, infoLogLength, NULL, &infoLog[0]);
		std::cerr << "Error compiling shader '" << shaderPath << "' : " << std::endl << &infoLog[0] << std::endl;
	}

	// Returning the shader ID.
	return shaderID;
}