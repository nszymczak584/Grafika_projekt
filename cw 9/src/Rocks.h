#pragma once

#include <vector>
#include "Render_Utils.h"
#include "Texture.h"

struct Rock {
    Core::RenderContext model;
    GLuint texture;
    GLuint normalMap;
};

void initRocks();
void generateRocks();
void drawRocks();