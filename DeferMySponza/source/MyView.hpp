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


class MyView : public tygra::WindowViewDelegate
{
public:
    MyView();
    ~MyView();

    void setScene(const sponza::Context * sponza);
	void ToggleSkybox();

private:
	const sponza::Context * mScene;
	ShaderProgram mSkyboxShaderProgram;
	ShaderProgram mAmbShaderProgram;
	ShaderProgram mDirShaderProgram;
	ShaderProgram mPointShaderProgram;
	ShaderProgram mSpotShaderProgram;
	std::vector<PerModelUniforms> mPerModelUniforms;
	std::map<sponza::MeshId, MeshData> mMeshes;
	MeshData mSkyboxMesh;
	std::map<std::string, GLuint> mTextures;
	bool mRenderSkybox = false;

    void windowViewWillStart(tygra::Window * window) override;
    void windowViewDidReset(tygra::Window * window, int width, int height) override;
    void windowViewDidStop(tygra::Window * window) override;
    void windowViewRender(tygra::Window * window) override;
	void DrawMeshesInstanced(ShaderProgram& shaderProgram) const;
	void LoadTexture(std::string path, std::string name);
	void LoadTextureCube(std::string path, std::string name);
};



