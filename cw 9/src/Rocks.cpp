#include "Rocks.h"
#include "Shader_Loader.h"
#include "Terrain.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <gtc/matrix_transform.hpp>

const std::vector<std::string> rockModelPaths = {
    "./models/rocks/rock_large_1.obj",
    "./models/rocks/rock_large_2.obj",
    "./models/rocks/rock_medium_3.obj",
    "./models/rocks/rock_medium_4.obj",
    "./models/rocks/rock_small_1.obj",
    "./models/rocks/rock_small_2.obj",
    "./models/rocks/rock_small_3.obj",
    "./models/rocks/rock_small_4.obj"
};

const std::vector<std::string> rockTexturePaths = {
    "./textures/rocks/rock_large_1.png",
    "./textures/rocks/rock_large_2.png",
    "./textures/rocks/rock_medium_3.png",
    "./textures/rocks/rock_medium_4.png",
    "./textures/rocks/rock_small.png",
    "./textures/rocks/rock_small.png",
    "./textures/rocks/rock_small.png",
    "./textures/rocks/rock_small.png"
};

const std::vector<std::string> rockNormalMapPaths = {
    "./textures/rocks/rock_large_1_normal.png",
    "./textures/rocks/rock_large_2_normal.png",
    "./textures/rocks/rock_medium_3_normal.png",
    "./textures/rocks/rock_medium_4_normal.png",
    "./textures/rocks/rock_small_normal.png",
    "./textures/rocks/rock_small_normal.png",
    "./textures/rocks/rock_small_normal.png",
    "./textures/rocks/rock_small_normal.png"
};

std::vector<Rock> rockModels;
std::vector<glm::mat4> rockTransforms;

extern void drawObjectTexturedNormal(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapID, float ambientLight = 0.2f);

void initRocks() {
    // Wczytanie modeli i tekstur ska³
    for (size_t i = 0; i < rockModelPaths.size(); ++i) {
        Rock rock;
        loadModelToContext(rockModelPaths[i].c_str(), rock.model);
        rock.texture = Core::LoadTexture(rockTexturePaths[i].c_str());
        rock.normalMap = Core::LoadTexture(rockNormalMapPaths[i].c_str());
        rockModels.push_back(rock);
    }

    generateRocks();
}

void generateRocks() {
    srand(static_cast<unsigned int>(time(nullptr)));

    const int numRocks = 500;
    rockTransforms.clear();

    for (int i = 0; i < numRocks; ++i) {
        float x = static_cast<float>(rand() % terrainSize) - terrainSize / 2;
        float z = static_cast<float>(rand() % terrainSize) - terrainSize / 2;
        float y = generateHeight(x, z) - 0.1f;  // Lekko pod powierzchni¹

        // Losowa rotacja w osi Y
        float angle = glm::radians(static_cast<float>(rand() % 360));

        // Losowe skalowanie (ska³y nie powinny byæ identyczne)
        float scale = 0.5f + static_cast<float>(rand() % 100) / 200.0f; // Zakres 0.5 - 1.0

        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z)) *
            glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(scale));

        rockTransforms.push_back(modelMatrix);
    }
}

void drawRocks() {
    for (size_t i = 0; i < rockTransforms.size(); ++i) {
        Rock& rock = rockModels[i % rockModels.size()];
        drawObjectTexturedNormal(rock.model, rockTransforms[i], rock.texture, rock.normalMap);
    }
}
