#include "BoidInteraction.h"
#include "glew.h"
#include "Render_Utils.h"
#include "ext.hpp"
#include "Shader_Loader.h"

bool isTargetActive = false;
bool isEscapeActive = false;

glm::vec3 targetPosition(0.0);
glm::vec3 escapePosition(0.0);

extern glm::vec3 cameraPos;
extern glm::vec3 cameraDir;
extern int WIDTH, HEIGHT;
extern GLuint program;
extern Core::RenderContext sphereContext;
extern glm::mat4 createPerspectiveMatrix();
extern glm::mat4 createCameraMatrix();

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

    return cameraPos + ray_world * 5.0f; // Pozycja 5 jednostek od kamery wzd³u¿ promienia
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

void renderInteractionIndicators(void (*drawFunction)(Core::RenderContext&, glm::mat4, glm::vec3, float, float)) {
    // Niebieska kula dla przyci¹gania
    if (isTargetActive) 
        drawFunction(sphereContext, glm::translate(targetPosition) * glm::scale(glm::vec3(0.1f)), glm::vec3(0.0f, 0.0f, 1.0f), 0.3f, 0.0f); 
    
    // Czerwona kula dla odpychania
    if (isEscapeActive) 
        drawFunction(sphereContext, glm::translate(escapePosition) * glm::scale(glm::vec3(0.1f)), glm::vec3(1.0f, 0.0f, 0.0f), 0.3f, 0.0f); 
}