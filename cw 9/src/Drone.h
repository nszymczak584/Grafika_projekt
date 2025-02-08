#pragma once

#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <vector>
#include <string>
#include <assimp/scene.h>
#include "Render_Utils.h"

namespace models {
    extern std::vector<Core::RenderContext> droneContexts;
}

extern glm::vec3 dronePos;
extern glm::vec3 droneDir;
extern glm::vec3 droneScale;

extern glm::vec3 cameraPos;
extern glm::vec3 cameraDir;

void loadDroneModel(const std::string& path);
void updateDrone(GLFWwindow* window, float deltaTime);
void updateCamera();
void drawDrone(void (*drawFunction)(Core::RenderContext&, glm::mat4, glm::vec3, float, float));