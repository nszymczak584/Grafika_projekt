#pragma once

#include "glew.h"
#include "glm.hpp"

// Sta³e definiuj¹ce teren
extern const float flatAreaSize;
extern const float flatAreaHeight;
extern const int terrainSize;
extern const float terrainScale;

// Deklaracje funkcji
void initTerrain();
void generateTerrain();
void drawTerrain( GLuint depthMap, glm::vec3 sunPos, float near_plane, float far_plane);
float generateHeight(float x, float z);