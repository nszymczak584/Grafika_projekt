#pragma once

#include <vector>
#include "glm.hpp"
#include "Render_Utils.h"

void generateTrees();
void drawTrees();
void initTrees();

extern std::vector<glm::vec3> treePositions;
extern std::vector<std::vector<glm::mat4>> treeLeavesOffsets;
extern Core::RenderContext treeContext;
extern Core::RenderContext leavesContext;
extern GLuint treeBarkTexture, treeBarkNormalMap;
extern GLuint treeLeavesTexture, treeLeavesNormalMap;
extern const int terrainSize;
extern const float flatAreaSize;