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

		glGenBuffers(1, &screen_quad_mesh_.vertex_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, screen_quad_mesh_.vertex_vbo);
		glBufferData(GL_ARRAY_BUFFER,
			vertices.size() * sizeof(glm::vec2),
			vertices.data(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &screen_quad_mesh_.vao);
		glBindVertexArray(screen_quad_mesh_.vao);
		glBindBuffer(GL_ARRAY_BUFFER, screen_quad_mesh_.vertex_vbo);
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

	/*
	* Tutorial: this code creates a cone to use when deferred shading
	*           with a spot light source.
	*/
	{
		tsl::IndexedMeshPtr mesh = tsl::createConePtr(1.f, 1.f, 12);
		mesh = tsl::cloneIndexedMeshAsTriangleListPtr(mesh.get());

		light_cone_mesh_.element_count = mesh->indexCount();

		glGenBuffers(1, &light_cone_mesh_.vertex_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, light_cone_mesh_.vertex_vbo);
		glBufferData(GL_ARRAY_BUFFER,
			mesh->vertexCount() * sizeof(glm::vec3),
			mesh->positionArray(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &light_cone_mesh_.element_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cone_mesh_.element_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			mesh->indexCount() * sizeof(unsigned int),
			mesh->indexArray(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &light_cone_mesh_.vao);
		glBindVertexArray(light_cone_mesh_.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cone_mesh_.element_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, light_cone_mesh_.vertex_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec3), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}



	// Terminating the program if 'scene_' is null.
    assert(mScene != nullptr);
	
	mGBufferShaderProgram.Init("resource:///gbuffer_vs.glsl", "resource:///gbuffer_fs.glsl", true);
	mGBufferShaderProgram.CreateUniformBuffer("cpp_PerFrameUniforms", sizeof(PerFrameUniforms), 0);
	mGBufferShaderProgram.CreateUniformBuffer("cpp_PerModelUniforms", sizeof(PerModelUniforms), 1);

	mAmbientShaderProgram.Init("resource:///deffered_vs.glsl", "resource:///ambient_fs.glsl");

	mDirectionalShaderProgram.Init("resource:///deffered_vs.glsl", "resource:///directional_fs.glsl");

	mPointShaderProgram.Init("resource:///point_vs.glsl", "resource:///point_fs.glsl");

	mSpotShaderProgram.Init("resource:///spot_vs.glsl", "resource:///spot_fs.glsl");

	mSSRShaderProgram.Init("resource:///deffered_vs.glsl", "resource:///ssr_fs.glsl");
	
	// Load the mesh data.
	sponza::GeometryBuilder geometryBuilder;
	for (const auto& mesh : geometryBuilder.getAllMeshes())
	{
		mMeshes[mesh.getId()].Init(mesh);
	}

	// Setting OpenGL to cull mesh faces.
	glEnable(GL_CULL_FACE);


	//------------------------------------------------------------------
	glGenTextures(1, &gbuffer_position_tex_);
	glGenTextures(1, &gbuffer_normal_tex_);
	glGenTextures(1, &gbuffer_colour_tex_);
	glGenTextures(1, &gbuffer_material_tex_);
	glGenTextures(1, &gbuffer_depth_tex_);
	glGenFramebuffers(1, &gbuffer_fbo_);
	glGenTextures(1, &lbuffer_colour_tex_);
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

	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_colour_tex_);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_RECTANGLE, gbuffer_colour_tex_, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_material_tex_);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_RECTANGLE, gbuffer_material_tex_, 0);

	GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, buffers);

	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);






	glBindFramebuffer(GL_FRAMEBUFFER, lbuffer_fbo_);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, gbuffer_depth_tex_, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, lbuffer_colour_tex_);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, lbuffer_colour_tex_, 0);

	GLenum buffers0[1] = { GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, buffers0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);






	glBindFramebuffer(GL_FRAMEBUFFER, postprocess_fbo_);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, gbuffer_depth_tex_, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, postprocess_colour_tex_);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, postprocess_colour_tex_, 0);

	GLenum buffers1[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers1);

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
	mDirectionalShaderProgram.Dispose();
	mPointShaderProgram.Dispose();
	mSpotShaderProgram.Dispose();

	//----------------------------------------------------------------
	glDeleteTextures(1, &gbuffer_position_tex_);
	glDeleteTextures(1, &gbuffer_normal_tex_);
	glDeleteTextures(1, &gbuffer_colour_tex_);
	glDeleteTextures(1, &gbuffer_depth_tex_);
	glDeleteTextures(1, &gbuffer_material_tex_);
	glDeleteFramebuffers(1, &gbuffer_fbo_);	
	glDeleteTextures(1, &lbuffer_colour_tex_);
	glDeleteFramebuffers(1, &lbuffer_fbo_);
	glDeleteTextures(1, &postprocess_colour_tex_);
	glDeleteFramebuffers(1, &postprocess_fbo_);
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
		mPerModelUniforms[meshID] = currentPerModelUniforms;
	}


	// CREATING THE GBUFFER
	//-----------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_fbo_);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

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

	mGBufferShaderProgram.SetUniformBuffer("cpp_PerFrameUniforms", &perFrameUniforms, sizeof(perFrameUniforms));
	//DrawMeshesInstanced(mGBufferShaderProgram);

	for (const auto& mesh : mMeshes)
	{
		glUniform1i(glGetUniformLocation(mGBufferShaderProgram.mProgramID, "cpp_EnableSSR"), (int)(mesh.second.GetMeshID() == 311));
		DrawMeshInstanced(mesh.second, mGBufferShaderProgram);
	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//-----------------------------------------------------------------


	// BIND AND CONFIGURE THE LBUFFER
	//-----------------------------------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, lbuffer_fbo_);
	glClearColor(0.f, 0.f, 0.25f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	//-----------------------------------------------------------------


	// AMBIENT
	//-----------------------------------------------------------------
	mAmbientShaderProgram.Use();

	glUniform1i(glGetUniformLocation(mDirectionalShaderProgram.mProgramID, "cpp_ColourTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_colour_tex_);

	auto ambientIntensity = (const glm::vec3 &)mScene->getAmbientLightIntensity();
	glUniform3fv(glGetUniformLocation(mAmbientShaderProgram.mProgramID, "cpp_AmbientIntensity"), 1, glm::value_ptr(ambientIntensity));

	glBindVertexArray(screen_quad_mesh_.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);	
	//-----------------------------------------------------------------


	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);


	// DIRECTIONAL
	//-----------------------------------------------------------------
	mDirectionalShaderProgram.Use();
	auto directionalLights = mScene->getAllDirectionalLights();
	for (auto light : directionalLights)
	{
		auto lightDir = (const glm::vec3 &)light.getDirection();
		auto lightIntensity = (const glm::vec3 &)light.getIntensity();

		glUniform1i(glGetUniformLocation(mDirectionalShaderProgram.mProgramID, "cpp_NormalTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_normal_tex_);

		glUniform3fv(glGetUniformLocation(mDirectionalShaderProgram.mProgramID, "cpp_LightDir"), 1, glm::value_ptr(lightDir));
		glUniform3fv(glGetUniformLocation(mDirectionalShaderProgram.mProgramID, "cpp_LightIntensity"), 1, glm::value_ptr(lightIntensity));

		glBindVertexArray(screen_quad_mesh_.vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glBindVertexArray(0);
	}	
	//-----------------------------------------------------------------


	// POINT
	//-----------------------------------------------------------------
	mPointShaderProgram.Use();

	glCullFace(GL_FRONT);

	glUniform1i(glGetUniformLocation(mPointShaderProgram.mProgramID, "cpp_PositionTex"), 0);
	glUniform1i(glGetUniformLocation(mPointShaderProgram.mProgramID, "cpp_NormalTex"), 1);
	glUniform1i(glGetUniformLocation(mPointShaderProgram.mProgramID, "cpp_ColourTex"), 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_position_tex_);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_normal_tex_);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_colour_tex_);

	auto pointLights = mScene->getAllPointLights();
	for (auto light : pointLights)
	{
		auto lightPos = (const glm::vec3 &)light.getPosition();
		auto lightIntensity = (const glm::vec3 &)light.getIntensity();
		auto lightRange = light.getRange();

		auto t = glm::translate(glm::mat4(), lightPos);
		auto s = glm::scale(glm::mat4(), glm::vec3(lightRange, lightRange, lightRange));
		auto model = t * s;
		auto mvp = projection * view * model;

		glUniformMatrix4fv(glGetUniformLocation(mPointShaderProgram.mProgramID, "cpp_MVP"), 1, false, glm::value_ptr(mvp));

		glUniform3fv(glGetUniformLocation(mPointShaderProgram.mProgramID, "cpp_LightPos"), 1, glm::value_ptr(lightPos));
		glUniform3fv(glGetUniformLocation(mPointShaderProgram.mProgramID, "cpp_LightIntensity"), 1, glm::value_ptr(lightIntensity));
		glUniform1f(glGetUniformLocation(mPointShaderProgram.mProgramID, "cpp_LightRange"), lightRange);

		glBindVertexArray(light_sphere_mesh_.vao);
		glDrawElements(GL_TRIANGLES, light_sphere_mesh_.element_count, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	glCullFace(GL_BACK);
	//-----------------------------------------------------------------


	// SPOT
	//-----------------------------------------------------------------
	mSpotShaderProgram.Use();

	glCullFace(GL_FRONT);

	glUniform1i(glGetUniformLocation(mSpotShaderProgram.mProgramID, "cpp_PositionTex"), 0);
	glUniform1i(glGetUniformLocation(mSpotShaderProgram.mProgramID, "cpp_NormalTex"), 1);
	glUniform1i(glGetUniformLocation(mSpotShaderProgram.mProgramID, "cpp_ColourTex"), 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_position_tex_);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_normal_tex_);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_colour_tex_);

	auto spotLights = mScene->getAllSpotLights();
	for (auto light : spotLights)
	{
		auto lightPos = (const glm::vec3 &)light.getPosition();
		auto lightIntensity = (const glm::vec3 &)light.getIntensity();
		auto lightRange = light.getRange();
		auto lightAngle = glm::radians(light.getConeAngleDegrees()) / 2.f;
		auto lightDir = glm::normalize((const glm::vec3 &)light.getDirection());

		auto t = glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -1.f));
		auto radius = glm::tan(lightAngle) * lightRange;
		auto s = glm::scale(glm::mat4(), glm::vec3(radius, radius, lightRange));
		auto r = glm::inverse(glm::lookAt(lightPos, lightPos + lightDir, glm::vec3(0.f, 1.f, 0.f)));
		auto model = r * s * t;
		auto mvp = projection * view * model;

		glUniformMatrix4fv(glGetUniformLocation(mSpotShaderProgram.mProgramID, "cpp_MVP"), 1, false, glm::value_ptr(mvp));

		glUniform3fv(glGetUniformLocation(mSpotShaderProgram.mProgramID, "cpp_LightPos"), 1, glm::value_ptr(lightPos));
		glUniform3fv(glGetUniformLocation(mSpotShaderProgram.mProgramID, "cpp_LightIntensity"), 1, glm::value_ptr(lightIntensity));
		glUniform1f(glGetUniformLocation(mSpotShaderProgram.mProgramID, "cpp_LightRange"), lightRange);
		glUniform1f(glGetUniformLocation(mSpotShaderProgram.mProgramID, "cpp_LightAngle"), lightAngle);
		glUniform3fv(glGetUniformLocation(mSpotShaderProgram.mProgramID, "cpp_LightDir"), 1, glm::value_ptr(lightDir));

		glBindVertexArray(light_cone_mesh_.vao);
		glDrawElements(GL_TRIANGLES, light_cone_mesh_.element_count, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	glCullFace(GL_BACK);
	//-----------------------------------------------------------------
	
	//Copying the lbuffer to the postprocess buffer.
	glBindFramebuffer(GL_READ_FRAMEBUFFER, lbuffer_fbo_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocess_fbo_);
	glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);




	// SSR
	//-----------------------------------------------------------------
	mSSRShaderProgram.Use();

	//glDisable(GL_BLEND);

	glUniform1i(glGetUniformLocation(mSSRShaderProgram.mProgramID, "cpp_PositionTex"), 0);
	glUniform1i(glGetUniformLocation(mSSRShaderProgram.mProgramID, "cpp_NormalTex"), 1);
	glUniform1i(glGetUniformLocation(mSSRShaderProgram.mProgramID, "cpp_ColourTex"), 2);
	glUniform1i(glGetUniformLocation(mSSRShaderProgram.mProgramID, "cpp_MaterialTex"), 3);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_position_tex_);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_normal_tex_);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, lbuffer_colour_tex_);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer_material_tex_);

	glUniform2fv(glGetUniformLocation(mSSRShaderProgram.mProgramID, "cpp_WindowDims"), 1, glm::value_ptr(glm::vec2(mWidth, mHeight)));
	glUniform3fv(glGetUniformLocation(mSSRShaderProgram.mProgramID, "cpp_CameraPos"), 1, glm::value_ptr(perFrameUniforms.cameraPos));
	glUniformMatrix4fv(glGetUniformLocation(mSSRShaderProgram.mProgramID, "cpp_ViewProjectionMatrix"), 1, false, glm::value_ptr(projection * view));	

	glBindVertexArray(screen_quad_mesh_.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
	//-----------------------------------------------------------------


	// BLITTING THE LBUFFER TO THE SCREEN
	//-----------------------------------------------------------------
	glBindFramebuffer(GL_READ_FRAMEBUFFER, postprocess_fbo_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	//-----------------------------------------------------------------

	


	// SHADING THE SCENE
	//-----------------------------------------------------------------
	/*glBindFramebuffer(GL_FRAMEBUFFER, lbuffer_fbo_);
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
	glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);*/
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
		shaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(mPerModelUniforms.at(meshID)), sizeof(mPerModelUniforms.at(meshID)));
		
		// Binding the mesh and drawing it.
		mesh.second.BindVAO();
		glDrawElementsInstanced(GL_TRIANGLES, mesh.second.GetElementCount(), GL_UNSIGNED_INT, 0, (GLsizei)instanceCount);
		glBindVertexArray(0);

		i++;
	}
}

void MyView::DrawMeshInstanced(const MeshData& mesh, ShaderProgram& shaderProgram) const
{
	const auto meshID = mesh.GetMeshID();
	const auto instanceIDs = mScene->getInstancesByMeshId(meshID);
	const auto instanceCount = instanceIDs.size();

	// Populating the per model and texture uniform variables for the current mesh.
	shaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(mPerModelUniforms.at(meshID)), sizeof(mPerModelUniforms.at(meshID)));

	// Binding the mesh and drawing it.
	mesh.BindVAO();

	glDrawElementsInstanced(GL_TRIANGLES, mesh.GetElementCount(), GL_UNSIGNED_INT, 0, (GLsizei)instanceCount);
	glBindVertexArray(0);
}