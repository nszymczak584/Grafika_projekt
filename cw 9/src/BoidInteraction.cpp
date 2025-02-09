#include "BoidInteraction.h"
#include "glew.h"
#include "Render_Utils.h"
#include "ext.hpp"
#include "Shader_Loader.h"
#include "Texture.h"
#include "Camera.h"


bool isTargetActive = false;
bool isEscapeActive = false;

glm::vec3 targetPosition(0.0);
glm::vec3 escapePosition(0.0);
float targetDistance = 5.0f;

extern glm::vec3 cameraPos;
extern glm::vec3 cameraDir;
extern int WIDTH, HEIGHT;
extern GLuint program;

GLuint lava, lavaNormalMap;
GLuint moss, mossNormalMap;
Core::RenderContext sphereContext;

extern void drawObjectTexturedNormal(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapID, float ambientLight = 0.2f);
extern void loadModelToContext(std::string path, Core::RenderContext& context);

void initInteractionSpheres() {
    loadModelToContext("./models/sphere.obj", sphereContext);
    
    lava = Core::LoadTexture("textures/lava/lava.jpg");
    lavaNormalMap = Core::LoadTexture("textures/lava/lava_normal.png");
    moss = Core::LoadTexture("textures/moss/moss.jpg");
    mossNormalMap = Core::LoadTexture("textures/moss/moss_normal.png");
}

glm::vec3 getMouseWorldPosition(GLFWwindow* window) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // Przekszta³cenie pozycji myszy do przestrzeni NDC (-1 do 1)
    float x = (2.0f * mouseX) / WIDTH - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / HEIGHT; // Oœ Y jest odwrócona w GLFW

    // Rzutowanie do przestrzeni oka
    glm::vec4 ray_clip(x, y, -1.0f, 1.0f);
    glm::vec4 ray_eye = glm::inverse(createPerspectiveMatrix()) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    // Rzutowanie do przestrzeni œwiata
    glm::vec3 ray_world = glm::vec3(glm::inverse(createCameraMatrix()) * ray_eye);
    ray_world = glm::normalize(ray_world);

    return cameraPos + ray_world * targetDistance;
}

void handleBoidInteraction(GLFWwindow* window, std::vector<Boid>& boids) {
    bool leftPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool rightPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    // Jeœli oba przyciski s¹ wciœniête - brak interakcji
    if (leftPressed && rightPressed) {
        isTargetActive = isEscapeActive = false;
        return;
    }

    glm::vec3 position;
    glm::vec3 forceDirection;
    float forceStrength = 0.05f;

    // Lewy przycisk myszy (przyci¹ganie)
    if (leftPressed) {
        position = targetPosition = getMouseWorldPosition(window);
        forceDirection = glm::vec3(1.0f); // Si³a skierowana w stronê celu
        isTargetActive = true;
        isEscapeActive = false;
    }
    // Prawy przycisk myszy (odpychanie)
    else if (rightPressed) {
        position = escapePosition = getMouseWorldPosition(window);
        forceDirection = glm::vec3(-1.0f); // Si³a skierowana przeciwnie do celu
        isEscapeActive = true;
        isTargetActive = false;
    }
    else {
        // Jeœli ¿aden przycisk nie jest wciœniêty - brak interakcji
        isTargetActive = isEscapeActive = false;
        return;
    }

    // Aktualizacja pozycji boidów
    for (auto& boid : boids) {
        glm::vec3 force = glm::normalize((position - boid.getPosition()) * forceDirection) * forceStrength;
        boid.applyForce(force);
    } 
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    float zoomSpeed = 0.1f; // Szybkoœæ zoomu
    float minDistance = 0.5f;  // Minimalna odleg³oœæ od kamery
    float maxDistance = 5.0f; // Maksymalna odleg³oœæ od kamery

    targetDistance += static_cast<float>(yoffset) * zoomSpeed;

    // Ograniczenie zakresu zoomowania
    if (targetDistance < minDistance) targetDistance = minDistance;
    if (targetDistance > maxDistance) targetDistance = maxDistance;
}

void drawInteractionSpheres(){
    // Niebieska kula (przyci¹ganie) - lawa
    if (isTargetActive) {
        drawObjectTexturedNormal(
            sphereContext,
            glm::translate(targetPosition) * glm::scale(glm::vec3(0.1f)),
            moss, mossNormalMap, 0.2f);
    }

    // Czerwona kula (odpychanie) - mech
    if (isEscapeActive) {
        drawObjectTexturedNormal(
            sphereContext,
            glm::translate(escapePosition) * glm::scale(glm::vec3(0.1f)),
            lava, lavaNormalMap, 0.2f);
    }
}