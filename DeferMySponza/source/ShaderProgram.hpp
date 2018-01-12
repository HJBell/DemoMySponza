#pragma once

#include <tgl/tgl.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>



class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();

	void Use() const;
	GLuint GetID() const;
	void Init(std::string vertexShaderPath, std::string fragmentShaderPath, bool isGBufferSahderProg = false);
	void CreateUniformBuffer(std::string name, GLsizeiptr size, int index);
	void SetUniformBuffer(std::string name, const void * data, GLsizeiptr size) const;
	void SetTextureUniform(GLuint textureID, std::string uniformName) const;
	void SetTextureCubeUniform(GLuint textureID, std::string uniformName) const;
	void Dispose();

private:
	GLuint mProgramID;
	std::unordered_map<std::string, GLuint> mUniformBuffers;

	GLuint LoadShader(std::string shaderPath, GLuint shaderType) const;
};

