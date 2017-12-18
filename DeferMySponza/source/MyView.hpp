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

private:
	const sponza::Context * mScene;
	ShaderProgram mGBufferShaderProgram;
	std::vector<PerModelUniforms> mPerModelUniforms;
	std::map<sponza::MeshId, MeshData> mMeshes;

	int mWidth = 0;
	int mHeight = 0;

	GLuint gbuffer_position_tex_{ 0 };
	GLuint gbuffer_normal_tex_{ 0 };
	GLuint gbuffer_depth_tex_{ 0 };
	GLuint gbuffer_fbo_{ 0 };

    void windowViewWillStart(tygra::Window * window) override;
    void windowViewDidReset(tygra::Window * window, int width, int height) override;
    void windowViewDidStop(tygra::Window * window) override;
    void windowViewRender(tygra::Window * window) override;
	void DrawMeshesInstanced(ShaderProgram& shaderProgram) const;
};



