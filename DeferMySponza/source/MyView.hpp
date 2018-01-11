#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <map>
#include "ShaderProgram.hpp"
#include "MeshData.hpp"
#include "UniformStructs.hpp"

enum TogglableFeature
{
	DirectionalLight, PointLights, SpotLights, ScreenSpaceReflection, AntiAliasing, Skybox
};

class MyView : public tygra::WindowViewDelegate
{
public:
    MyView();
    ~MyView();

    void setScene(const sponza::Context * sponza);
	void ToggleFeature(TogglableFeature feature);


private:
	int mWidth = 0;
	int mHeight = 0;

	std::map<TogglableFeature, bool> mFeatureIsEnabled;

	const sponza::Context * mScene;

	std::map<sponza::MeshId, PerModelUniforms> mPerModelUniforms;
	PerFrameUniforms mPerFrameUniforms;
	std::map<std::string, GLuint> mTextures;

	std::map<sponza::MeshId, MeshData> mMeshes;
	MeshData mSkyboxMesh;	

	struct Mesh
	{
		GLuint vertex_vbo{ 0 };
		GLuint element_vbo{ 0 };
		GLuint vao{ 0 };
		int element_count{ 0 };
	};

	Mesh screen_quad_mesh_; // vertex array of vec2 position
	Mesh light_sphere_mesh_; // element array into vec3 position
	Mesh light_cone_mesh_;

	ShaderProgram mSkyboxShaderProgram;
	ShaderProgram mGBufferShaderProgram;
	ShaderProgram mAmbientShaderProgram;
	ShaderProgram mDirectionalShaderProgram;
	ShaderProgram mPointShaderProgram;
	ShaderProgram mSpotShaderProgram;
	ShaderProgram mSSRShaderProgram;
	ShaderProgram mAAShaderProgram;

	GLuint gbuffer_position_tex_{ 0 };
	GLuint gbuffer_normal_tex_{ 0 };
	GLuint gbuffer_colour_tex_{ 0 };
	GLuint gbuffer_depth_tex_{ 0 };
	GLuint gbuffer_material_tex_{ 0 };
	GLuint gbuffer_fbo_{ 0 };

	GLuint lbuffer_fbo_{ 0 };
	GLuint lbuffer_colour_tex_{ 0 };

	GLuint postprocess_fbo_{ 0 };
	GLuint postprocess_colour_tex_{ 0 };

    void windowViewWillStart(tygra::Window * window) override;
    void windowViewDidReset(tygra::Window * window, int width, int height) override;
    void windowViewDidStop(tygra::Window * window) override;
    void windowViewRender(tygra::Window * window) override;
	void DrawMeshesInstanced(const ShaderProgram& shaderProgram) const;
	void DrawMeshInstanced(const MeshData& mesh, const ShaderProgram& shaderProgram) const;
	void LoadTexture(std::string path, std::string name);
	void LoadTextureCube(std::string path, std::string name);

	void RenderGBuffer() const;

	void RenderSkybox(GLuint targetFBO, const glm::vec3& cameraPos, const glm::mat4& vpMatrix) const;
	void RenderAmbientLight(GLuint targetFBO, GLuint colTex) const;
	void RenderDirectionalLights(GLuint targetFBO, GLuint normTex) const;
	void RenderPointLights(GLuint targetFBO, GLuint colTex, GLuint posTex, GLuint normTex, const glm::mat4& vpMatrix) const;
	void RenderSpotLights(GLuint targetFBO, GLuint colTex, GLuint posTex, GLuint normTex, const glm::mat4& vpMatrix) const;

	void PostProc_SSR(GLuint targetFBO, GLuint colTex, GLuint posTex, GLuint normTex, GLuint matTex, const glm::vec3& cameraPos, const glm::mat4& vpMatrix) const;
	void PostProc_AA(GLuint targetFBO, GLuint colTex) const;
};



