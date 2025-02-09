#include "Tree.h"
#include "Texture.h"
#include "Shader_Loader.h"
#include <cstdlib>
#include <ctime>
#include <gtc/matrix_transform.hpp>

std::vector<glm::vec3> treePositions;
std::vector<std::vector<glm::mat4>> treeLeavesOffsets;

Core::RenderContext treeContext;
Core::RenderContext leavesContext;

GLuint treeBarkTexture, treeBarkNormalMap;
GLuint treeLeavesTexture, treeLeavesNormalMap;

extern float generateHeight(float x, float z);
extern const int terrainSize;

extern void loadModelToContext(std::string path, Core::RenderContext& context);
extern void drawObjectTexturedNormal(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapID, float ambientLight = 0.2f);
extern const float flatAreaSize;

void initTrees() {
    loadModelToContext("./models/MapleTree.obj", treeContext);
    loadModelToContext("./models/MapleTreeLeaves.obj", leavesContext);

    treeBarkTexture = Core::LoadTexture("textures/tree/bark.jpg");
    treeBarkNormalMap = Core::LoadTexture("textures/tree/bark_normal.png");
    treeLeavesTexture = Core::LoadTexture("textures/tree/leaf.png");
    treeLeavesNormalMap = Core::LoadTexture("textures/tree/leaf_normal.png");
    generateTrees();
}

void generateTrees() {
    treePositions.clear();
    treeLeavesOffsets.clear();

    srand(static_cast<unsigned int>(time(nullptr)));

    for (int i = 0; i < 100; ++i) {
        float x = static_cast<float>(rand() % terrainSize) - terrainSize / 2;
        float z = static_cast<float>(rand() % terrainSize) - terrainSize / 2;

        // **Sprawdzenie, czy drzewo nie znajduje siê w obszarze flatArea**
        if (x >= -flatAreaSize / 2 && x <= flatAreaSize / 2 &&
            z >= -flatAreaSize / 2 && z <= flatAreaSize / 2) {
            continue; // Pomijamy ten punkt, aby unikn¹æ sadzenia drzew na p³askim obszarze
        }

        float y = generateHeight(x, z);
        treePositions.push_back(glm::vec3(x, y, z));
    }

    // Generowanie losowych przesuniêæ dla liœci
    for (size_t i = 0; i < treePositions.size(); ++i) {
        std::vector<glm::mat4> offsets;
        for (int j = 0; j < 3; j++) {
            glm::mat4 offsetMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(
                (rand() % 10 - 5) * 0.02f,
                (rand() % 10 - 5) * 0.02f,
                (rand() % 10 - 5) * 0.02f
            )) * glm::rotate(glm::mat4(1.0f), glm::radians(float(rand() % 360)), glm::vec3(0.0f, 1.0f, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(1.1f));

            offsets.push_back(offsetMatrix);
        }
        treeLeavesOffsets.push_back(offsets);
    }
}

void drawTrees() {
    for (size_t i = 0; i < treePositions.size(); ++i) {
        glm::mat4 treeModelMatrix = glm::translate(glm::mat4(), treePositions[i]) * glm::scale(glm::mat4(), glm::vec3(0.2));

        drawObjectTexturedNormal(treeContext, treeModelMatrix, treeBarkTexture, treeBarkNormalMap);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        for (const auto& offset : treeLeavesOffsets[i]) {
            glm::mat4 modifiedTreeModelMatrix = treeModelMatrix * offset;
            drawObjectTexturedNormal(leavesContext, modifiedTreeModelMatrix, treeLeavesTexture, treeLeavesNormalMap);
        }

        glDisable(GL_CULL_FACE);
    }
}