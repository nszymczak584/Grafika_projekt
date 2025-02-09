#include "Drone.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "Texture.h"
#include "Shader_Loader.h"

namespace models {
    std::vector<Core::RenderContext> droneContexts;
}

float lastTime = -1.f;
float deltaTime = 0.f;

std::vector<GLuint> droneTextures;
std::vector<GLuint> droneNormalMaps;


glm::vec3 dronePos = glm::vec3(-6.0f, 2.8f, -6.8f);
glm::vec3 droneDir = glm::vec3(0.6f, 0.f, 0.8f);
glm::vec3 droneScale = glm::vec3(0.15f);

glm::vec3 cameraPos = dronePos - droneDir * 1.5f + glm::vec3(0, 0.5f, 0);
glm::vec3 cameraDir = glm::normalize(droneDir);

extern glm::vec3 sunDir;

extern void drawObjectTexturedNormal(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapID, float ambientLight = 0.2f);

void loadDroneModel(const std::string& path) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        std::string meshName = scene->mMeshes[i]->mName.C_Str();

        // Ignorowanie okreœlonych submeshów
        if (meshName.find("Cube.002_Cube.004") != std::string::npos ||
            meshName.find("Sphere") != std::string::npos ||
            meshName.find("Cylinder.001") != std::string::npos ||
            meshName.find("Cube.001_Cube.002") != std::string::npos) {
            //std::cout << "Ignoring submesh: " << meshName << std::endl;
            continue;
        }

        //std::cout << "Loading submesh: " << meshName << std::endl;

        // Tworzenie render contextu
        Core::RenderContext context;
        context.initFromAssimpMesh(scene->mMeshes[i]);
        models::droneContexts.push_back(context);

        // Przypisanie tekstur do konkretnego submesha
        if (meshName.find("Cylinder.005_Cylinder.006") != std::string::npos) {
            droneTextures.push_back(Core::LoadTexture("./textures/drone/metal_gold/metal_gold.jpg"));
            droneNormalMaps.push_back(Core::LoadTexture("./textures/drone/metal_gold/metal_gold_normal.png"));
        }
        else if (meshName.find("Cylinder.006_Cylinder.007") != std::string::npos) {
            droneTextures.push_back(Core::LoadTexture("./textures/drone/metal_grey/metal_grey.jpg"));
            droneNormalMaps.push_back(Core::LoadTexture("./textures/drone/metal_grey/metal_grey_normal.png"));
        }
        else {
            // Reszta submeshów dostaje metal_black
            droneTextures.push_back(Core::LoadTexture("./textures/drone/metal_black/metal_black.jpg"));
            droneNormalMaps.push_back(Core::LoadTexture("./textures/drone/metal_black/metal_black_normal.png"));
        }
    }
}

void updateDeltaTime() {
    static float lastTime = -1.f;
    float time = glfwGetTime();
    if (lastTime < 0) {
        lastTime = time;
        return;
    }

    deltaTime = time - lastTime;
    if (deltaTime > 0.1) deltaTime = 0.1;
    lastTime = time;
}

void initDrone() {
    loadDroneModel("./models/drone.obj");
}

void updateDrone(GLFWwindow* window) {
    glm::vec3 droneSide = glm::normalize(glm::cross(droneDir, glm::vec3(0.f, 1.f, 0.f)));
    float angleSpeed = 1.5f * deltaTime;
    float moveSpeed = 6.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) dronePos.y += moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) dronePos.y -= moveSpeed;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        droneDir = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), angleSpeed, glm::vec3(0, 1, 0)) * glm::vec4(droneDir, 0.0f)));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        droneDir = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), -angleSpeed, glm::vec3(0, 1, 0)) * glm::vec4(droneDir, 0.0f)));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dronePos += droneDir * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dronePos -= droneDir * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) dronePos -= droneSide * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) dronePos += droneSide * moveSpeed;
}

void updateCamera() {
    cameraPos = dronePos - droneDir * 1.5f + glm::vec3(0, 0.5f, 0);
    cameraDir = glm::normalize(droneDir);
}

void drawDrone() {
    glm::vec3 droneSide = glm::normalize(glm::cross(droneDir, glm::vec3(0.f, 1.f, 0.f)));
    glm::vec3 droneUp = glm::normalize(glm::cross(droneSide, droneDir));

    glm::mat4 droneRotation = glm::mat4({
        droneSide.x, droneSide.y, droneSide.z, 0,
        droneUp.x, droneUp.y, droneUp.z, 0,
        -droneDir.x, -droneDir.y, -droneDir.z, 0,
        0., 0., 0., 1.0f,
        });

    glm::mat4 correctionRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
    glm::mat4 modelMatrix = glm::translate(dronePos) * droneRotation * correctionRotation * glm::scale(droneScale);

    for (size_t i = 0; i < models::droneContexts.size(); i++) {
        if (i >= droneTextures.size() || i >= droneNormalMaps.size()) {
            std::cout << "Warning: Missing texture for submesh " << i << std::endl;
            continue;
        }

        GLuint textureID = droneTextures[i];
        GLuint normalID = droneNormalMaps[i];

        drawObjectTexturedNormal(models::droneContexts[i], modelMatrix, textureID, normalID);
    }

    updateDeltaTime();
}
