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



void MyView::ToggleFeature(TogglableFeature feature)
{
	// Toggling the specified feature of the renderer.
	mFeatureIsEnabled[feature] = !mFeatureIsEnabled[feature];
}



//------------------------------------Private Functions-------------------------------------

void MyView::windowViewWillStart(tygra::Window * window)
{
	// Generating the quad mesh to be used for full screen rendering.
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

	// Generating the sphere mesh to be used in the point light rendering.
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

	// Generating the cone mesh to be used in the spot light rendering.
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
	
	// Initialising the shader programs.
	mSkyboxShaderProgram.Init("resource:///skybox_vs.glsl", "resource:///skybox_fs.glsl");
	mSkyboxShaderProgram.CreateUniformBuffer("cpp_SkyboxUniforms", sizeof(SkyboxUniforms), 2);
	mGBufferShaderProgram.Init("resource:///gbuffer_vs.glsl", "resource:///gbuffer_fs.glsl", true);
	mGBufferShaderProgram.CreateUniformBuffer("cpp_PerFrameUniforms", sizeof(PerFrameUniforms), 0);
	mGBufferShaderProgram.CreateUniformBuffer("cpp_PerModelUniforms", sizeof(PerModelUniforms), 1);
	mAmbientShaderProgram.Init("resource:///deffered_vs.glsl", "resource:///ambient_fs.glsl");
	mDirectionalShaderProgram.Init("resource:///deffered_vs.glsl", "resource:///directional_fs.glsl");
	mPointShaderProgram.Init("resource:///point_vs.glsl", "resource:///point_fs.glsl");
	mSpotShaderProgram.Init("resource:///spot_vs.glsl", "resource:///spot_fs.glsl");
	mSSRShaderProgram.Init("resource:///deffered_vs.glsl", "resource:///ssr_fs.glsl");
	mAAShaderProgram.Init("resource:///deffered_vs.glsl", "resource:///aa_fs.glsl");
	
	// Load the mesh data.
	sponza::GeometryBuilder geometryBuilder;
	for (const auto& mesh : geometryBuilder.getAllMeshes())
	{
		mMeshes[mesh.getId()].Init(mesh);
	}

	// Initialising the skybox mesh.
	mSkyboxMesh.Init(CubeVerts);

	// Loading textures.
	LoadTexture("resource:///marble.png", "Marble");
	LoadTextureCube("resource:///skybox_stormy_", "Skybox");

	// Generating the OpenGL textures and frame buffer objects.
	glGenTextures(1, &gbuffer_position_tex_);
	glGenTextures(1, &gbuffer_normal_tex_);
	glGenTextures(1, &gbuffer_colour_tex_);
	glGenTextures(1, &gbuffer_material_tex_);
	glGenTextures(1, &gbuffer_depth_tex_);
	glGenFramebuffers(1, &gbuffer_fbo_);
	glGenTextures(1, &lbuffer_colour_tex_);
	glGenFramebuffers(1, &lbuffer_fbo_);
	
	// Setting the renderer features to their initial on/off state.
	mFeatureIsEnabled[DirectionalLight] = true;
	mFeatureIsEnabled[PointLights] = true;
	mFeatureIsEnabled[SpotLights] = true;
	mFeatureIsEnabled[ScreenSpaceReflection] = false;
	mFeatureIsEnabled[AntiAliasing] = false;
	mFeatureIsEnabled[Skybox] = false;
}



void MyView::windowViewDidReset(tygra::Window * window, int width, int height)
{
	// Recording the screen width and height.
	mWidth = width;
	mHeight = height;

	// Setting the OpenGL viewport position and size.
    glViewport(0, 0, mWidth, mHeight);

	// Setting up the gbuffer textures and frame buffer object.
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


	// Setting up the lbuffer textures and frame buffer object.
	glBindFramebuffer(GL_FRAMEBUFFER, lbuffer_fbo_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, gbuffer_depth_tex_, 0);
	glBindTexture(GL_TEXTURE_RECTANGLE, lbuffer_colour_tex_);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, lbuffer_colour_tex_, 0);
	GLenum buffers0[1] = { GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, buffers0);
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);

	// Setting up the post process textures and frame buffer object.
	glBindFramebuffer(GL_FRAMEBUFFER, postprocess_fbo_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, gbuffer_depth_tex_, 0);
	glBindTexture(GL_TEXTURE_RECTANGLE, postprocess_colour_tex_);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, postprocess_colour_tex_, 0);
	GLenum buffers1[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers1);
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
}



void MyView::windowViewDidStop(tygra::Window * window)
{
	// Disposing of the mesh data.
	for (auto& mesh : mMeshes)
		mesh.second.Dispose();

	// Disposing of the shader programs.
	mSkyboxShaderProgram.Dispose();
	mGBufferShaderProgram.Dispose();
	mAmbientShaderProgram.Dispose();
	mDirectionalShaderProgram.Dispose();
	mPointShaderProgram.Dispose();
	mSpotShaderProgram.Dispose();
	mSSRShaderProgram.Dispose();
	mAAShaderProgram.Dispose();

	// Deleting the loaded textures.
	for (const auto& tex : mTextures)
		glDeleteTextures(1, &tex.second);

	// Deleting the OpenGL textures and frame buffer objects.
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
}



void MyView::windowViewRender(tygra::Window * window)
{
	// Terminating the program if 'scene_' is null.
	assert(mScene != nullptr);

	// Calculating the aspect ratio.
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspectRatio = viewportSize[2] / (float)viewportSize[3];

	// Getting the camera data for the frame.
	sponza::Camera camera = mScene->getCamera();
	auto camDir = (const glm::vec3 &)camera.getDirection();
	auto upDir = (const glm::vec3 &)mScene->getUpDirection();

	// Populating the per frame uniform buffer.
	mPerFrameUniforms.cameraPos = (const glm::vec3 &)camera.getPosition();
	mPerFrameUniforms.ambientIntensity = (const glm::vec3 &)mScene->getAmbientLightIntensity();

	// Calculating the projection and view matrices.
	glm::mat4 projection = glm::perspective(glm::radians(camera.getVerticalFieldOfViewInDegrees()),
		aspectRatio, camera.getNearPlaneDistance(),
		camera.getFarPlaneDistance());	
	glm::mat4 view = glm::lookAt(mPerFrameUniforms.cameraPos, mPerFrameUniforms.cameraPos + camDir, upDir);

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

	// Rendering the GBuffer.
	RenderGBuffer();

	// Clearing the LBuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, lbuffer_fbo_);
	glClearColor(0.42f, 0.15f, 0.53f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Rendering the skybox if enabled.
	if(mFeatureIsEnabled[Skybox]) RenderSkybox(lbuffer_fbo_, mPerFrameUniforms.cameraPos, projection * view);
	
	// Carrying out the amient pass.	
	RenderAmbientLight(lbuffer_fbo_, gbuffer_colour_tex_);

	// Carrying out the directional pass if enabled.
	if (mFeatureIsEnabled[DirectionalLight]) RenderDirectionalLights(lbuffer_fbo_, gbuffer_normal_tex_);

	// Carrying out the point light pass if enabled.
	if (mFeatureIsEnabled[PointLights]) RenderPointLights(lbuffer_fbo_, gbuffer_colour_tex_, gbuffer_position_tex_, gbuffer_normal_tex_, projection * view);

	// Carrying out the spot light pass if enabled.
	if (mFeatureIsEnabled[SpotLights]) RenderSpotLights(lbuffer_fbo_, gbuffer_colour_tex_, gbuffer_position_tex_, gbuffer_normal_tex_, projection * view);
	
	// Copying the lbuffer to the postprocess buffer ready to be blended onto.
	glBindFramebuffer(GL_READ_FRAMEBUFFER, lbuffer_fbo_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocess_fbo_);
	glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// Adding screen-space reflections if enabled.
	if (mFeatureIsEnabled[ScreenSpaceReflection])
	{
		PostProc_SSR(postprocess_fbo_, lbuffer_colour_tex_, gbuffer_position_tex_, gbuffer_normal_tex_, gbuffer_material_tex_, mPerFrameUniforms.cameraPos, projection * view);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, postprocess_fbo_);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lbuffer_fbo_);
		glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	// Adding anti aliasing if enabled.
	if (mFeatureIsEnabled[AntiAliasing]) PostProc_AA(postprocess_fbo_, lbuffer_colour_tex_);
	
	// Blitting to the screen
	glBindFramebuffer(GL_READ_FRAMEBUFFER, postprocess_fbo_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}



void MyView::DrawMeshInstanced(const MeshData& mesh, const ShaderProgram& shaderProgram) const
{
	// Getting the instance count for the mesh.
	const auto meshID = mesh.GetMeshID();
	const auto instanceIDs = mScene->getInstancesByMeshId(meshID);
	const auto instanceCount = instanceIDs.size();

	// Populating the per model and texture uniform variables for the current mesh.
	shaderProgram.SetUniformBuffer("cpp_PerModelUniforms", &(mPerModelUniforms.at(meshID)), sizeof(mPerModelUniforms.at(meshID)));
	shaderProgram.SetTextureUniform(mTextures.at("Marble"), "cpp_Texture");

	// Binding the mesh and drawing it.
	mesh.BindVAO();
	glDrawElementsInstanced(GL_TRIANGLES, mesh.GetElementCount(), GL_UNSIGNED_INT, 0, (GLsizei)instanceCount);
	glBindVertexArray(0);
}



void MyView::LoadTexture(std::string path, std::string name)
{
	//Checking the texture is not already loaded.
	if (mTextures.find(path) != mTextures.end()) return;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//Loading the texture.
	tygra::Image texture = tygra::createImageFromPngFile(path);

	//Checking the texture contains data.
	if (texture.doesContainData())
	{
		//Loading the texture and storing its ID in the 'textures' map.
		glGenTextures(1, &mTextures[name]);
		glBindTexture(GL_TEXTURE_2D, mTextures[name]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			(GLsizei)texture.width(),
			(GLsizei)texture.height(),
			0,
			pixel_formats[texture.componentsPerPixel()],
			texture.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE
			: GL_UNSIGNED_SHORT,
			texture.pixelData());
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else std::cerr << "Warning : Texture '" << path << "' does not contain any data." << std::endl;
}



void MyView::LoadTextureCube(std::string path, std::string name)
{
	// Checking the texture is not already loaded.
	if (mTextures.find(path) != mTextures.end()) return;

	// Generating the texture cube.
	glGenTextures(1, &mTextures[name]);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTextures[name]);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Loading each texture to be used as a face for the texture cube.
	for (size_t i = 0; i < 6; ++i) {
		const std::string url = path + std::to_string(i) + ".png";
		tygra::Image img = tygra::createImageFromPngFile(url);
		if (!img.doesContainData()) {
			throw std::runtime_error("failed to load " + url);
		}
		GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
		glTexImage2D((GLenum)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, GL_RGBA, (GLsizei)img.width(), (GLsizei)img.height(), 0,
			pixel_formats[img.componentsPerPixel()],
			img.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT, img.pixelData());
	}
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}



void MyView::RenderGBuffer() const
{
	// Binding the gbuffer frame buffer object.
	glBindFramebuffer(GL_FRAMEBUFFER, gbuffer_fbo_);

	// Using the gbuffer shader program.
	mGBufferShaderProgram.Use();

	// Configuring OpenGL.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 0, ~0);
	glStencilMask(~0);
	glClearStencil(128);

	// Setting the shader 'per frame' uniform variables.
	mGBufferShaderProgram.SetUniformBuffer("cpp_PerFrameUniforms", &mPerFrameUniforms, sizeof(mPerFrameUniforms));

	// Looping through the meshes in the scene.
	for (const auto& mesh : mMeshes)
	{
		// Setting the shader uniforms.
		glUniform1i(glGetUniformLocation(mGBufferShaderProgram.GetID(), "cpp_EnableSSR"), (int)(mesh.second.GetMeshID() == 311));

		// Drawing the instances of the mesh.
		DrawMeshInstanced(mesh.second, mGBufferShaderProgram);
	}
}



void MyView::RenderSkybox(GLuint targetFBO, const glm::vec3& cameraPos, const glm::mat4& vpMatrix) const
{
	// Binding the target frame buffer object.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

	// Using the skybox shader program.
	mSkyboxShaderProgram.Use();

	// Configuring OpenGL.
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glActiveTexture(GL_TEXTURE0);

	// Populating the skybox shaders uniform variables.
	SkyboxUniforms skyboxUniforms;
	skyboxUniforms.cameraPos = cameraPos;
	skyboxUniforms.viewProjectionXform = vpMatrix;
	mSkyboxShaderProgram.SetUniformBuffer("cpp_SkyboxUniforms", &skyboxUniforms, sizeof(skyboxUniforms));
	mSkyboxShaderProgram.SetTextureCubeUniform(mTextures.at("Skybox"), "cpp_CubeMap");

	// Binding the skybox VAO and drawing.
	mSkyboxMesh.BindVAO();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	// Configuring OpenGL for subsequent draws.
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}



void MyView::RenderAmbientLight(GLuint targetFBO, GLuint colTex) const
{
	// Binding the target frame buffer object.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

	// Using the ambient shader program.
	mAmbientShaderProgram.Use();

	// Configuring OpenGL.
	glDisable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// Setting the shader uniform variables.
	glUniform1i(glGetUniformLocation(mDirectionalShaderProgram.GetID(), "cpp_ColourTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, colTex);
	auto ambientIntensity = (const glm::vec3 &)mScene->getAmbientLightIntensity();
	glUniform3fv(glGetUniformLocation(mAmbientShaderProgram.GetID(), "cpp_AmbientIntensity"), 1, glm::value_ptr(ambientIntensity));

	// Binding the quad VAO and drawing.
	glBindVertexArray(screen_quad_mesh_.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}



void MyView::RenderDirectionalLights(GLuint targetFBO, GLuint normTex) const
{
	// Binding the target frame buffer object.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

	// Using the directional shader program.
	mDirectionalShaderProgram.Use();

	// Configuring OpenGL.
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// Looping through the directional lights in the scene.
	auto directionalLights = mScene->getAllDirectionalLights();
	for (auto light : directionalLights)
	{
		// Setting the shader uniform variables.
		auto lightDir = (const glm::vec3 &)light.getDirection();
		auto lightIntensity = (const glm::vec3 &)light.getIntensity();
		glUniform1i(glGetUniformLocation(mDirectionalShaderProgram.GetID(), "cpp_NormalTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE, normTex);
		glUniform3fv(glGetUniformLocation(mDirectionalShaderProgram.GetID(), "cpp_LightDir"), 1, glm::value_ptr(lightDir));
		glUniform3fv(glGetUniformLocation(mDirectionalShaderProgram.GetID(), "cpp_LightIntensity"), 1, glm::value_ptr(lightIntensity));

		// Binding the quad VAO and drawing.
		glBindVertexArray(screen_quad_mesh_.vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glBindVertexArray(0);
	}
}



void MyView::RenderPointLights(GLuint targetFBO, GLuint colTex, GLuint posTex, GLuint normTex, const glm::mat4& vpMatrix) const
{
	// Binding the target frame buffer object.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

	// Using the point light shader program.
	mPointShaderProgram.Use();

	// Configuring OpenGL.
	glCullFace(GL_FRONT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// Passing the uniform textures to the shader.
	glUniform1i(glGetUniformLocation(mPointShaderProgram.GetID(), "cpp_PositionTex"), 0);
	glUniform1i(glGetUniformLocation(mPointShaderProgram.GetID(), "cpp_NormalTex"), 1);
	glUniform1i(glGetUniformLocation(mPointShaderProgram.GetID(), "cpp_ColourTex"), 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, posTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, normTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, colTex);

	// Looping through the point lights in the scene.
	auto pointLights = mScene->getAllPointLights();
	for (auto light : pointLights)
	{
		// Setting the shader uniform variables.
		auto lightPos = (const glm::vec3 &)light.getPosition();
		auto lightIntensity = (const glm::vec3 &)light.getIntensity();
		auto lightRange = light.getRange();
		auto t = glm::translate(glm::mat4(), lightPos);
		auto s = glm::scale(glm::mat4(), glm::vec3(lightRange, lightRange, lightRange));
		auto model = t * s;
		auto mvp = vpMatrix * model;
		glUniformMatrix4fv(glGetUniformLocation(mPointShaderProgram.GetID(), "cpp_MVP"), 1, false, glm::value_ptr(mvp));
		glUniform3fv(glGetUniformLocation(mPointShaderProgram.GetID(), "cpp_LightPos"), 1, glm::value_ptr(lightPos));
		glUniform3fv(glGetUniformLocation(mPointShaderProgram.GetID(), "cpp_LightIntensity"), 1, glm::value_ptr(lightIntensity));
		glUniform1f(glGetUniformLocation(mPointShaderProgram.GetID(), "cpp_LightRange"), lightRange);

		// Binding the sphere VAO and drawing.
		glBindVertexArray(light_sphere_mesh_.vao);
		glDrawElements(GL_TRIANGLES, light_sphere_mesh_.element_count, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}



void MyView::RenderSpotLights(GLuint targetFBO, GLuint colTex, GLuint posTex, GLuint normTex, const glm::mat4& vpMatrix) const
{
	// Binding the target frame buffer object.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

	// Using the spot light shader program.
	mSpotShaderProgram.Use();

	// Configuring OpenGL.
	glCullFace(GL_FRONT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// Passing the uniform textures to the shader.
	glUniform1i(glGetUniformLocation(mSpotShaderProgram.GetID(), "cpp_PositionTex"), 0);
	glUniform1i(glGetUniformLocation(mSpotShaderProgram.GetID(), "cpp_NormalTex"), 1);
	glUniform1i(glGetUniformLocation(mSpotShaderProgram.GetID(), "cpp_ColourTex"), 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, posTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, normTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, colTex);

	// Looping through the spot lights in the scene.
	auto spotLights = mScene->getAllSpotLights();
	for (auto light : spotLights)
	{
		// Setting the shader uniform variables.
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
		auto mvp = vpMatrix * model;
		glUniformMatrix4fv(glGetUniformLocation(mSpotShaderProgram.GetID(), "cpp_MVP"), 1, false, glm::value_ptr(mvp));
		glUniform3fv(glGetUniformLocation(mSpotShaderProgram.GetID(), "cpp_LightPos"), 1, glm::value_ptr(lightPos));
		glUniform3fv(glGetUniformLocation(mSpotShaderProgram.GetID(), "cpp_LightIntensity"), 1, glm::value_ptr(lightIntensity));
		glUniform1f(glGetUniformLocation(mSpotShaderProgram.GetID(), "cpp_LightRange"), lightRange);
		glUniform1f(glGetUniformLocation(mSpotShaderProgram.GetID(), "cpp_LightAngle"), lightAngle);
		glUniform3fv(glGetUniformLocation(mSpotShaderProgram.GetID(), "cpp_LightDir"), 1, glm::value_ptr(lightDir));

		// Binding the cone VAO and drawing.
		glBindVertexArray(light_cone_mesh_.vao);
		glDrawElements(GL_TRIANGLES, light_cone_mesh_.element_count, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}



void MyView::PostProc_SSR(GLuint targetFBO, GLuint colTex, GLuint posTex, GLuint normTex, GLuint matTex, const glm::vec3& cameraPos, const glm::mat4& vpMatrix) const
{
	// Binding the target frame buffer object.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

	// Using the SSR shader program.
	mSSRShaderProgram.Use();

	// Configuring OpenGL.
	glEnable(GL_BLEND);
	glCullFace(GL_BACK);

	// Setting the shader uniform variables.
	glUniform1i(glGetUniformLocation(mSSRShaderProgram.GetID(), "cpp_PositionTex"), 0);
	glUniform1i(glGetUniformLocation(mSSRShaderProgram.GetID(), "cpp_NormalTex"), 1);
	glUniform1i(glGetUniformLocation(mSSRShaderProgram.GetID(), "cpp_ColourTex"), 2);
	glUniform1i(glGetUniformLocation(mSSRShaderProgram.GetID(), "cpp_MaterialTex"), 3);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, posTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, normTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, colTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_RECTANGLE, matTex);
	glUniform2fv(glGetUniformLocation(mSSRShaderProgram.GetID(), "cpp_WindowDims"), 1, glm::value_ptr(glm::vec2(mWidth, mHeight)));
	glUniform3fv(glGetUniformLocation(mSSRShaderProgram.GetID(), "cpp_CameraPos"), 1, glm::value_ptr(cameraPos));
	glUniformMatrix4fv(glGetUniformLocation(mSSRShaderProgram.GetID(), "cpp_ViewProjectionMatrix"), 1, false, glm::value_ptr(vpMatrix));

	// Binding the quad VAO and drawing.
	glBindVertexArray(screen_quad_mesh_.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}



void MyView::PostProc_AA(GLuint targetFBO, GLuint colTex) const
{
	// Binding the target frame buffer object.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

	// Using the AA shader program.
	mAAShaderProgram.Use();

	// Configuring OpenGL.
	glDisable(GL_BLEND);
	glCullFace(GL_BACK);

	// Setting the shader uniform variables.
	glUniform1i(glGetUniformLocation(mAAShaderProgram.GetID(), "cpp_ColourTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, colTex);

	// Binding the quad VAO and drawing.
	glBindVertexArray(screen_quad_mesh_.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}