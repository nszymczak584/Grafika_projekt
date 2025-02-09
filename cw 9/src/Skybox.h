#pragma once

#include "glew.h"
#include <vector>
#include <string>
#include "glm.hpp"
#include "Shader_Loader.h"
#include "Render_Utils.h"

extern void loadModelToContext(std::string path, Core::RenderContext& context);

extern glm::mat4 createCameraMatrix();
extern glm::mat4 createPerspectiveMatrix();

GLuint loadCubemap(const std::vector<std::string>& faces);
void drawSkybox();
void initSkybox();