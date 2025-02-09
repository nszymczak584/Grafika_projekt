#include "NormalMapToggle.h"
#include "Texture.h"

bool useNormalMapping = true;

float lastToggleTime = 0.0f;
const float toggleCooldown = 0.5f;

GLuint flatNormalMap;

void handleNormalMapToggle(GLFWwindow* window) {
    float currentTime = glfwGetTime();

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && (currentTime - lastToggleTime) > toggleCooldown) {
        useNormalMapping = !useNormalMapping;
        lastToggleTime = currentTime;
        std::cout << "Normal Mapping: " << (useNormalMapping ? "ON" : "OFF") << std::endl;
    }
}

void initNormalMapToggle() {
    flatNormalMap = Core::LoadTexture("textures/flat_normal.png");
}