#pragma once

#include <sponza/sponza_fwd.hpp>
#include <glm/glm.hpp>
#include <tgl/tgl.h>
#include <map>
#include <vector>

class MeshData
{
public:
	MeshData();
	~MeshData();

	void Init(const sponza::Mesh& mesh);
	void Init(const std::vector<glm::vec3>& vertPositions);
	void BindVAO() const;
	GLsizei GetElementCount() const;
	void Dispose();

private:
	GLuint mVAO = 0;
	std::map<std::string, GLuint> mVBOs;
	size_t mElementCount = 0;
	
	GLuint GenerateBuffer(const void* data, GLsizeiptr size, GLenum bufferType, GLenum drawType) const;
};

