#pragma once

#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <vector>
#include <string>
#include <assimp/scene.h>
#include "Render_Utils.h"
#include "Texture.h"

namespace models {
    extern std::vector<Core::RenderContext> droneContexts;
}

extern glm::vec3 dronePos;
extern glm::vec3 droneDir;
extern glm::vec3 droneScale;

extern glm::vec3 cameraPos;
extern glm::vec3 cameraDir;


extern glm::mat4 createCameraMatrix();
extern glm::mat4 createPerspectiveMatrix();
extern void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, float roughness, float metallic);

void loadDroneModel(const std::string& path);
void updateDeltaTime();
void updateDrone(GLFWwindow* window);
void updateCamera();
void drawDrone();
void initDrone();