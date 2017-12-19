#include "MyView.hpp"
#include "Primitives.hpp"
#include <tsl/shapes.hpp>
#include <sponza/sponza.hpp>
#include <tygra/FileHelper.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cassert>


//-------------------------------------Public Functions-------------------------------------

MyView::MyView()
{	
}

MyView::~MyView() 
{
}

void MyView::setScene(const sponza::Context * sponza)
{
    mScene = sponza;
}


//------------------------------------Private Functions-------------------------------------

void MyView::windowViewWillStart(tygra::Window * window)
{
	/*
	* Tutorial: this section of code creates a fullscreen quad to be used
	*           when computing global illumination effects (e.g. ambient)
	*/
	{
		std::vector<glm::vec2> vertices(4);
		vertices[0] = glm::vec2(-1, -1);
		vertices[1] = glm::vec2(1, -1);
		vertices[2] = glm::vec2(1, 1);
		vertices[3] = glm::vec2(-1, 1);

		glGenBuffers(1, &light_quad_mesh_.vertex_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, light_quad_mesh_.vertex_vbo);
		glBufferData(GL_ARRAY_BUFFER,
			vertices.size() * sizeof(glm::vec2),
			vertices.data(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &light_quad_mesh_.vao);
		glBindVertexArray(light_quad_mesh_.vao);
		glBindBuffer(GL_ARRAY_BUFFER, light_quad_mesh_.vertex_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec2), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	/*
	* Tutorial: this code creates a sphere to use when deferred shading
	*           with a point light source.
	*/
	{
		tsl::IndexedMeshPtr mesh = tsl::createSpherePtr(1.f, 12);
		mesh = tsl::cloneIndexedMeshAsTriangleListPtr(mesh.get());

		light_sphere_mesh_.element_count = mesh->indexCount();

		glGenBuffers(1, &light_sphere_mesh_.vertex_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, light_sphere_mesh_.vertex_vbo);
		glBufferData(GL_ARRAY_BUFFER,
			mesh->vertexCount() * sizeof(glm::vec3),
			mesh->positionArray(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &light_sphere_mesh_.element_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_sphere_mesh_.element_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			mesh->indexCount() * sizeof(unsigned int),
			mesh->indexArray(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &light_sphere_mesh_.vao);
		glBindVertexArray(light_sphere_mesh_.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_sphere_mesh_.element_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, light_sphere_mesh_.vertex_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec3), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}




	// Terminating the program if 'scene_' is null.
    assert(mScene != nullptr);
	
	mGBufferShaderProgram.Init("resource:///sponza_vs.glsl", "resource:///gbuffer_fs.glsl", true);
	mGBufferShaderProgram.CreateUniformBuffer("cpp_PerFrameUniforms", sizeof(PerFrameUniforms), 0);
	mGBufferShaderProgram.CreateUniformBuffer("cpp_PerModelUniforms", sizeof(PerModelUniforms), 1);

	mAmbientShaderProgram.Init("resource:///global_light_vs.glsl", "resource:///global_light_fs.glsl");
	
	// Load the mesh data.
	sponza::GeometryBuilder geometryBuilder;
	for (const auto& mesh : geometryBuilder.getAllMeshes())
		mMeshes[mesh.getId()].Init(mesh);

	// Setting OpenGL to cull mesh faces.
	glEnable(GL_CULL_FACE);


	//------------------------------------------------------------------
	glGenTextures(1, &gbuffer_position_tex_);
	glGenTextures(1, &gbuffer_normal_tex_);
	glGenTextures(1, &gbuffer_depth_tex_);
	glGenFramebuffers(1, &gbuffer_fbo_);
	glGenRenderbuffers(1, &lbuffer_colour_rbo_);
	glGenFramebuffers(1, &lbuffer_fbo_);
	//------------------------------------------------------------------
}

void MyView::windowViewDidReset(tygra::Window * window, int width, int height)
{
	mWidth = width;
	mHeight = height;

	// Setting the OpenGL viewport position and size.
    glViewport(0, 0, mWidth, mHeight);

	// Specifying clear values for the colour buffers (0-1).
	glClearColor(0.f, 0.f, 0.25f, 0.f);


	//-----------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_fbo_);

	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_depth_tex_);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH24_STENCIL8, mWidth, mHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, gbuffer_depth_tex_, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_position_tex_);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, gbuffer_position_tex_, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_normal_tex_);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, gbuffer_normal_tex_, 0);

	GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, buffers);

	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);



	glBindRenderbuffer(GL_RENDERBUFFER, lbuffer_colour_rbo_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, mWidth, mHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, lbuffer_fbo_);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, lbuffer_colour_rbo_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, gbuffer_depth_tex_, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
	//-----------------------------------------------------------------
}

void MyView::windowViewDidStop(tygra::Window * window)
{
	// Disposing of the mesh data.
	for (auto& mesh : mMeshes)
		mesh.second.Dispose();

	// Disposing of the shader programs.
	mGBufferShaderProgram.Dispose();
	mAmbientShaderProgram.Dispose();

	//----------------------------------------------------------------
	glDeleteTextures(1, &gbuffer_position_tex_);
	glDeleteTextures(1, &gbuffer_normal_tex_);
	glDeleteTextures(1, &gbuffer_depth_tex_);
	glDeleteFramebuffers(1, &gbuffer_fbo_);	
	glDeleteFramebuffers(1, &lbuffer_fbo_);
	glDeleteRenderbuffers(1, &lbuffer_colour_rbo_);
	//----------------------------------------------------------------
}

void MyView::windowViewRender(tygra::Window * window)
{
	// Terminating the program if 'scene_' is null.
	assert(mScene != nullptr);

	// Calculating the aspect ratio.
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspectRatio = viewportSize[2] / (float)viewportSize[3];

	// Creating the per frame and per model structs.
	PerFrameUniforms perFrameUniforms;

	// Getting the camera data for the frame.
	sponza::Camera camera = mScene->getCamera();
	auto camDir = (const glm::vec3 &)camera.getDirection();
	auto upDir = (const glm::vec3 &)mScene->getUpDirection();

	// Populating the per frame uniform buffer.
	perFrameUniforms.cameraPos = (const glm::vec3 &)camera.getPosition();
	perFrameUniforms.ambientIntensity = (const glm::vec3 &)mScene->getAmbientLightIntensity();

	// Calculating the projection and view matrices.
	glm::mat4 projection = glm::perspective(glm::radians(camera.getVerticalFieldOfViewInDegrees()),
		aspectRatio, camera.getNearPlaneDistance(),
		camera.getFarPlaneDistance());	
	glm::mat4 view = glm::lookAt(perFrameUniforms.cameraPos, perFrameUniforms.cameraPos + camDir, upDir);

	// Looping through the meshes in the scene and capturing their uniform properties for the frame.
	mPerModelUniforms.clear();
	for (const auto& mesh : mMeshes)
	{
		const auto meshID = mesh.first;
		const auto instanceIDs = mScene->getInstancesByMeshId(meshID);
		const auto instanceCount = instanceIDs.size();

		// Creating a uniform buffer block to capture the uniform data for the current mesh.
		PerModelUniforms currentPerModelUniforms;

		// Loop through the instances and populate the uniform buffer block.
		for (int i = 0; i < instanceCount; i++)
		{
			auto instance = mScene->getInstanceById(instanceIDs[i]);

			// Setting the xforms in the uniform buffer.
			currentPerModelUniforms.instances[i].modelXform = (const glm::mat4x3 &)instance.getTransformationMatrix();
			currentPerModelUniforms.instances[i].mvpXform = projection * view * currentPerModelUniforms.instances[i].modelXform;

			// Setting the material properties in the uniform buffer.
			auto material = mScene->getMaterialById(instance.getMaterialId());
			currentPerModelUniforms.instances[i].diffuse = (const glm::vec3 &)material.getDiffuseColour();
			currentPerModelUniforms.instances[i].shininess = material.getShininess();
			currentPerModelUniforms.instances[i].specular = (const glm::vec3 &)material.getSpecularColour();
			currentPerModelUniforms.instances[i].isShiny = material.isShiny();
		}

		// Adding the current meshes uniform buffer to the mPerModelUniforms data structure.
		mPerModelUniforms.push_back(currentPerModelUniforms);
	}

	// CREATING THE GBUFFER
	//-----------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_fbo_);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	{
		mGBufferShaderProgram.Use();
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glClearStencil(128);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 0, ~0);
		glStencilMask(~0);

		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Populating the ambient shaders uniform variables.
		mGBufferShaderProgram.SetUniformBuffer("cpp_PerFrameUniforms", &perFrameUniforms, sizeof(perFrameUniforms));

		// Drawing the meshes in the scene (instanced).
		DrawMeshesInstanced(mGBufferShaderProgram);
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	//-----------------------------------------------------------------


	// SHADING THE SCENE
	//-----------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, lbuffer_fbo_);
	glClearColor(0.f, 0.f, 0.25f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	mAmbientShaderProgram.Use();

	glUniform1i(glGetUniformLocation(mAmbientShaderProgram.mProgramID, "sampler_world_position"), 0);
	glUniform1i(glGetUniformLocation(mAmbientShaderProgram.mProgramID, "sampler_world_normal"), 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_position_tex_);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_normal_tex_);
	const glm::vec3 global_light_direction = glm::normalize(glm::vec3(-3.f, -2.f, 1.f));
	glUniform3fv(glGetUniformLocation(mAmbientShaderProgram.mProgramID, "light_direction"), 1, glm::value_ptr(global_light_direction));

	glBindVertexArray(light_quad_mesh_.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	//-----------------------------------------------------------------
}

void MyView::DrawMeshesInstanced(ShaderProgram& shaderProgram) const
{
	// Looping through each mesh in the scene and drawing it (instanced).
	int i = 0;
	for (const auto& mesh : mMeshes)
	{
		const auto meshID = mesh.first;
		const auto instanceIDs = mScene->getInstancesByMeshId(meshID);
		const auto instanceCount = instanceIDs.size();

		// Populating the per model and texture uniform variables for the current mesh.
		shaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(mPerModelUniforms[i]), sizeof((mPerModelUniforms[i])));

		// Binding the mesh and drawing it.
		mesh.second.BindVAO();
		glDrawElementsInstanced(GL_TRIANGLES, mesh.second.GetElementCount(), GL_UNSIGNED_INT, 0, (GLsizei)instanceCount);
		glBindVertexArray(0);

		i++;
	}
}