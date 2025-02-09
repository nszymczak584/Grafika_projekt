#pragma once

#include "glew.h"
#include "glm.hpp"

// Sta�e definiuj�ce teren
extern const float flatAreaSize;
extern const float flatAreaHeight;
extern const int terrainSize;
extern const float terrainScale;

// Deklaracje funkcji
void initTerrain();
void generateTerrain();
void drawTerrain();
float generateHeight(float x, float z);