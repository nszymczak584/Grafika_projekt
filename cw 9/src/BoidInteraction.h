#pragma once

#include "Boid.h"
#include "glm.hpp"
#include "glew.h"
#include <GLFW/glfw3.h>
#include "Render_Utils.h"

extern bool isTargetActive;
extern bool isEscapeActive;

extern glm::vec3 targetPosition;
extern glm::vec3 escapePosition;

// Pobiera pozycj� myszy w �wiecie 3D 
glm::vec3 getMouseWorldPosition(GLFWwindow* window);

// Obs�uguje interakcj� myszy z boidami (przyci�ganie i odpychanie)
void handleBoidInteraction(GLFWwindow* window, std::vector<Boid>& boids, const std::vector<CollidableObject>& collidableObjects);

// Renderuje wska�niki interakcji (kule dla przyci�gania i odpychania)
void renderInteractionIndicators(void (*drawFunction)(Core::RenderContext&, glm::mat4, glm::vec3, float, float));