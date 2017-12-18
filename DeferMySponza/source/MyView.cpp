#include "MyView.hpp"
#include "Primitives.hpp"
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

void MyView::ToggleSkybox()
{
	mRenderSkybox = !mRenderSkybox;
}


//------------------------------------Private Functions-------------------------------------

void MyView::windowViewWillStart(tygra::Window * window)
{
	// Terminating the program if 'scene_' is null.
    assert(mScene != nullptr);

	// Creating the ambient pass shader program.
	mAmbShaderProgram.Init("resource:///sponza_vs.glsl", "resource:///ambient_fs.glsl");
	mAmbShaderProgram.CreateUniformBuffer("cpp_PerFrameUniforms", sizeof(PerFrameUniforms), 0);
	mAmbShaderProgram.CreateUniformBuffer("cpp_PerModelUniforms", sizeof(PerModelUniforms), 1);

	// Creating the direction light pass shader program.
	mDirShaderProgram.Init("resource:///sponza_vs.glsl", "resource:///dir_fs.glsl");
	mDirShaderProgram.CreateUniformBuffer("cpp_DirectionalLightUniforms", sizeof(DirectionalLightUniforms), 2);
	mDirShaderProgram.CreateUniformBuffer("cpp_PerFrameUniforms", sizeof(PerFrameUniforms), 3);
	mDirShaderProgram.CreateUniformBuffer("cpp_PerModelUniforms", sizeof(PerModelUniforms), 4);

	// Creating the point light pass shader program.
	mPointShaderProgram.Init("resource:///sponza_vs.glsl", "resource:///point_fs.glsl");
	mPointShaderProgram.CreateUniformBuffer("cpp_PointLightUniforms", sizeof(PointLightUniforms), 5);
	mPointShaderProgram.CreateUniformBuffer("cpp_PerFrameUniforms", sizeof(PerFrameUniforms), 6);
	mPointShaderProgram.CreateUniformBuffer("cpp_PerModelUniforms", sizeof(PerModelUniforms), 7);

	// Creating the spot light pass shader program.
	mSpotShaderProgram.Init("resource:///sponza_vs.glsl", "resource:///spot_fs.glsl");
	mSpotShaderProgram.CreateUniformBuffer("cpp_SpotLightUniforms", sizeof(SpotLightUniforms), 8);
	mSpotShaderProgram.CreateUniformBuffer("cpp_PerFrameUniforms", sizeof(PerFrameUniforms), 9);
	mSpotShaderProgram.CreateUniformBuffer("cpp_PerModelUniforms", sizeof(PerModelUniforms), 10);

	// Creating the skybox pass shader program.
	mSkyboxShaderProgram.Init("resource:///skybox_vs.glsl", "resource:///skybox_fs.glsl");
	mSkyboxShaderProgram.CreateUniformBuffer("cpp_SkyboxUniforms", sizeof(SkyboxUniforms), 11);
	
	// Load the mesh data.
	sponza::GeometryBuilder geometryBuilder;
	for (const auto& mesh : geometryBuilder.getAllMeshes())
		mMeshes[mesh.getId()].Init(mesh);

	// Initialising the skybox mesh.
	mSkyboxMesh.Init(CubeVerts);
	
	// Loading textures.
	LoadTexture("resource:///hex.png", "Hex");
	LoadTexture("resource:///marble.png", "Marble");
	LoadTextureCube("resource:///skybox_stormy_", "Skybox");

	// Setting OpenGL to cull mesh faces.
	glEnable(GL_CULL_FACE);
}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
	// Setting the OpenGL viewport position and size.
    glViewport(0, 0, width, height);

	// Specifying clear values for the colour buffers (0-1).
	glClearColor(0.f, 0.f, 0.25f, 0.f);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
	// Disposing of the mesh data.
	for (auto& mesh : mMeshes)
		mesh.second.Dispose();

	// Disposing of the shader programs.
	mSkyboxShaderProgram.Dispose();
	mAmbShaderProgram.Dispose();
	mDirShaderProgram.Dispose();
	mPointShaderProgram.Dispose();
	mSpotShaderProgram.Dispose();

	// Deleting the textures.
	for (const auto& tex : mTextures)
		glDeleteTextures(1, &tex.second);
}

void MyView::windowViewRender(tygra::Window * window)
{
	// Terminating the program if 'scene_' is null.
	assert(mScene != nullptr);

	// Clearing the contents of the buffers from the previous frame.
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	// Carrying out an initial pass to draw the skybox if required.
	if (mRenderSkybox)
	{
		// Setting the skybox shader program to be active and configuring OpenGL for the pass.
		mSkyboxShaderProgram.Use();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		// Populating the skybox shaders uniform variables.
		SkyboxUniforms skyboxUniforms;
		skyboxUniforms.cameraPos = (const glm::vec3 &)camera.getPosition();
		skyboxUniforms.viewProjectionXform = projection * view;
		mSkyboxShaderProgram.SetUniformBuffer("cpp_SkyboxUniforms", &skyboxUniforms, sizeof(skyboxUniforms));
		mSkyboxShaderProgram.SetTextureCubeUniform(mTextures["Skybox"], "cpp_CubeMap");

		// Binding the skybox VAO and drawing.
		mSkyboxMesh.BindVAO();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}	

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

	// Carrying out the ambient pass.
	{
		// Setting the ambient shader program to be active and configuring OpenGL for the pass.
		mAmbShaderProgram.Use();
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		// Populating the ambient shaders uniform variables.
		mAmbShaderProgram.SetUniformBuffer("cpp_PerFrameUniforms", &perFrameUniforms, sizeof(perFrameUniforms));

		// Drawing the meshes in the scene (instanced).
		DrawMeshesInstanced(mAmbShaderProgram);
	}
	
	// Carrying out the directional pass.
	{
		// Setting the directional shader program to be active and configuring OpenGL for the pass.
		mDirShaderProgram.Use();
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		// Populating the directional shaders per frame uniform variables.
		mDirShaderProgram.SetUniformBuffer("cpp_PerFrameUniforms", &perFrameUniforms, sizeof(perFrameUniforms));

		// Drawing the scene once for each directional light.
		for (const auto& light : mScene->getAllDirectionalLights())
		{
			// Populating the directional shaders light uniform variables.
			DirectionalLightUniforms directionalLightUniform;
			directionalLightUniform.light.direction = (const glm::vec3 &)light.getDirection();
			directionalLightUniform.light.intensity = (const glm::vec3 &)light.getIntensity();
			mDirShaderProgram.SetUniformBuffer("cpp_DirectionalLightUniforms", &directionalLightUniform, sizeof(directionalLightUniform));

			// Drawing the meshes in the scene (instanced).
			DrawMeshesInstanced(mDirShaderProgram);
		}
	}
	
	// Carrying out the point light pass.
	{
		// Setting the point light shader program to be active.
		mPointShaderProgram.Use();

		// Populating the point light shaders per frame uniform variables.
		mPointShaderProgram.SetUniformBuffer("cpp_PerFrameUniforms", &perFrameUniforms, sizeof(perFrameUniforms));

		// Drawing the scene once for each point light.
		for (const auto& light : mScene->getAllPointLights())
		{
			// Populating the point light shaders light uniform variables.
			PointLightUniforms pointLightUniform;
			pointLightUniform.light.position = (const glm::vec3 &)light.getPosition();
			pointLightUniform.light.range = light.getRange();
			pointLightUniform.light.intensity = (const glm::vec3 &)light.getIntensity();
			mPointShaderProgram.SetUniformBuffer("cpp_PointLightUniforms", &pointLightUniform, sizeof(pointLightUniform));

			// Drawing the meshes in the scene (instanced).
			DrawMeshesInstanced(mPointShaderProgram);
		}
	}

	// Carrying out the spot light pass.
	{
		// Setting the spot light shader program to be active.
		mSpotShaderProgram.Use();

		// Populating the spot light shaders per frame uniform variables.
		mSpotShaderProgram.SetUniformBuffer("cpp_PerFrameUniforms", &perFrameUniforms, sizeof(perFrameUniforms));

		// Drawing the scene once for each spot light.
		for (const auto& light : mScene->getAllSpotLights())
		{
			// Populating the spot light shaders light uniform variables.
			SpotLightUniforms spotLightUniform;
			spotLightUniform.light.position = (const glm::vec3 &)light.getPosition();
			spotLightUniform.light.range = light.getRange();
			spotLightUniform.light.intensity = (const glm::vec3 &)light.getIntensity();
			spotLightUniform.light.angle = light.getConeAngleDegrees();
			spotLightUniform.light.direction = (const glm::vec3 &)light.getDirection();
			mSpotShaderProgram.SetUniformBuffer("cpp_SpotLightUniforms", &spotLightUniform, sizeof(spotLightUniform));

			// Drawing the meshes in the scene (instanced).
			DrawMeshesInstanced(mSpotShaderProgram);
		}
	}
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
		shaderProgram.SetTextureUniform(mTextures.at("Marble"), "cpp_Texture");

		// Binding the mesh and drawing it.
		mesh.second.BindVAO();
		glDrawElementsInstanced(GL_TRIANGLES, mesh.second.GetElementCount(), GL_UNSIGNED_INT, 0, (GLsizei)instanceCount);
		glBindVertexArray(0);

		i++;
	}
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
