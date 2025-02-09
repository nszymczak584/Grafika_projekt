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


// Pobiera pozycjê myszy w œwiecie 3D 
glm::vec3 getMouseWorldPosition(GLFWwindow* window);

// Obs³uguje interakcjê myszy z boidami (przyci¹ganie i odpychanie)
void handleBoidInteraction(GLFWwindow* window, std::vector<Boid>& boids);

// Renderuje wskaŸniki interakcji (kule dla przyci¹gania i odpychania)
void drawInteractionSpheres();
void initInteractionSpheres();

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);