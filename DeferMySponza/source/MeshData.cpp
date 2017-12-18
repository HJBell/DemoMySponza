#include "MeshData.hpp"
#include <sponza/sponza.hpp>


//-------------------------------------Public Functions-------------------------------------

MeshData::MeshData()
{
}


MeshData::~MeshData()
{
}

void MeshData::Init(const sponza::Mesh& mesh)
{
	// Disposing of any VBOs/VAOs that may already exist.
	Dispose();

	// Getting the mesh components.
	const auto& positions = mesh.getPositionArray();
	const auto& normals = mesh.getNormalArray();
	const auto& elements = mesh.getElementArray();
	const auto& textureCoords = mesh.getTextureCoordinateArray();

	// Generating and binding the VAO.
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);	
	
	// Generating, binding and populating the element VBO.
	mVBOs["Elements"] = (GenerateBuffer(elements.data(), elements.size() * sizeof(unsigned int), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBOs["Elements"]);

	// Storing the element count.
	mElementCount = elements.size();

	// Generating, binding and populating the positions VBO.
	mVBOs["Positions"] = (GenerateBuffer(positions.data(), positions.size() * sizeof(glm::vec3), GL_ARRAY_BUFFER, GL_STATIC_DRAW));
	glBindBuffer(GL_ARRAY_BUFFER, mVBOs["Positions"]);

	// Assigning the positions VBO to vertex attrib array index 0.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Generating, binding and populating the normals VBO.
	mVBOs["Normals"] = (GenerateBuffer(normals.data(), normals.size() * sizeof(glm::vec3), GL_ARRAY_BUFFER, GL_STATIC_DRAW));
	glBindBuffer(GL_ARRAY_BUFFER, mVBOs["Normals"]);

	// Assigning the positions VBO to vertex attrib array index 1.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Creating the texture coordinate VBO if required.
	if (textureCoords.size() > 0)
	{
		// Generating, binding and populating the texture coordinate VBO.
		mVBOs["TexCoords"] = (GenerateBuffer(textureCoords.data(), textureCoords.size() * sizeof(glm::vec2), GL_ARRAY_BUFFER, GL_STATIC_DRAW));
		glBindBuffer(GL_ARRAY_BUFFER, mVBOs["TexCoords"]);

		// Assigning the texture coordinate VBO to vertex attrib array index 2.
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), TGL_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// Unbinding the VAO.
	glBindVertexArray(0);
}

void MeshData::Init(const std::vector<glm::vec3>& vertPositions)
{
	// Disposing of any VBOs/VAOs that may already exist.
	Dispose();

	// Generating and binding the VAO.
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);
	
	// Generating and binding the positions VBO.
	mVBOs["Positions"] = (GenerateBuffer(vertPositions.data(), vertPositions.size() * sizeof(glm::vec3), GL_ARRAY_BUFFER, GL_STATIC_DRAW));
	glBindBuffer(GL_ARRAY_BUFFER, mVBOs["Positions"]);

	// Assigning the positions VBO to vertex attrib array index 0.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Unbinding the VAO.
	glBindVertexArray(0);
}

void MeshData::BindVAO() const
{
	// Binding the VAO.
	glBindVertexArray(mVAO);
}

GLsizei MeshData::GetElementCount() const
{
	// Returning the element count as a GLsizei.
	return (GLsizei)mElementCount;
}

void MeshData::Dispose()
{
	// Looping through the VBOs and deleting any that are not set to 0.
	for (const auto& vbo : mVBOs)
		if(vbo.second != 0)
			glDeleteBuffers(1, &vbo.second);

	// Deleting the VAO if it is not set the 0.
	if (mVAO != 0)
		glDeleteVertexArrays(1, &mVAO);
}


//------------------------------------Private Functions-------------------------------------

GLuint MeshData::GenerateBuffer(const void* data, GLsizeiptr size, GLenum bufferType, GLenum drawType) const
{
	// Generating and binding a buffer object.
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(bufferType, buffer);

	// Populating the buffer object.
	glBufferData(bufferType, size, data, drawType);

	// Unbinding the buffer object.
	glBindBuffer(bufferType, 0);

	// Returning the buffer object.
	return buffer;
}