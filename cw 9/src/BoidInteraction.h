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


// Pobiera pozycję myszy w świecie 3D 
glm::vec3 getMouseWorldPosition(GLFWwindow* window);

// Obsługuje interakcję myszy z boidami (przyciąganie i odpychanie)
void handleBoidInteraction(GLFWwindow* window, std::vector<Boid>& boids, const std::vector<CollidableObject>& collidableObjects);

// Renderuje wskaźniki interakcji (kule dla przyciągania i odpychania)
void drawInteractionSpheres();
void initInteractionSpheres();

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);