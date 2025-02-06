#pragma once

#include "glew.h"
#include <vector>
#include <string>
#include "glm.hpp"
#include "Shader_Loader.h"
#include "Render_Utils.h"

extern GLuint skyboxTexture;
extern GLuint skyboxShader;
extern Core::RenderContext skyBoxContext;

extern std::vector<std::string> skyboxFaces;

GLuint loadCubemap(const std::vector<std::string>& faces);
void drawSkybox(glm::mat4 cameraMatrix, glm::mat4 perspectiveMatrix);
void initSkybox(Core::Shader_Loader& shaderLoader, void(*loadModelFunc)(std::string, Core::RenderContext&));