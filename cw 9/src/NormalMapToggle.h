#pragma once

#include "glew.h"
#include <GLFW/glfw3.h>
#include <iostream>

extern bool useNormalMapping;
extern GLuint flatNormalMap;

void handleNormalMapToggle(GLFWwindow* window);
void initNormalMapToggle();