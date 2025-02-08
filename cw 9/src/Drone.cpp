#include "drone.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

namespace models {
    std::vector<Core::RenderContext> droneContexts;
}

glm::vec3 dronePos = glm::vec3(12.7f, 2.8f, 2.0f);
glm::vec3 droneDir = glm::vec3(-0.95f, 0.0f, -0.28f);
glm::vec3 droneScale = glm::vec3(0.1f);

glm::vec3 cameraPos = dronePos - droneDir * 1.5f + glm::vec3(0, 0.5f, 0);
glm::vec3 cameraDir = glm::normalize(droneDir);

void (*drawFunction)(Core::RenderContext&, glm::mat4, glm::vec3, float, float);

void loadDroneModel(const std::string& path) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        std::string meshName = scene->mMeshes[i]->mName.C_Str();

        if (meshName.find("Cube.002_Cube.004") != std::string::npos || meshName.find("Sphere") != std::string::npos) {
            std::cout << "Ignoring submesh: " << meshName << std::endl;
            continue;
        }
        std::cout << "Loading submesh: " << meshName << std::endl;

        Core::RenderContext context;
        context.initFromAssimpMesh(scene->mMeshes[i]);
        models::droneContexts.push_back(context);
    }
}

void updateDrone(GLFWwindow* window, float deltaTime) {
    glm::vec3 droneSide = glm::normalize(glm::cross(droneDir, glm::vec3(0.f, 1.f, 0.f)));
    float angleSpeed = 1.5f * deltaTime;  // Zwiêkszona responsywnoœæ
    float moveSpeed = 3.0f * deltaTime;   // Zwiêkszona responsywnoœæ

    // Zmiana wysokoœci
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) dronePos.y += moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) dronePos.y -= moveSpeed;

    // Obrót yaw (obracanie w poziomie)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        droneDir = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), angleSpeed, glm::vec3(0, 1, 0)) * glm::vec4(droneDir, 0.0f)));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        droneDir = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), -angleSpeed, glm::vec3(0, 1, 0)) * glm::vec4(droneDir, 0.0f)));

    // Ruch przód/ty³ i na boki
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dronePos += droneDir * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dronePos -= droneDir * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) dronePos -= droneSide * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) dronePos += droneSide * moveSpeed;
}

void updateCamera() {
    cameraPos = dronePos - droneDir * 1.5f + glm::vec3(0, 0.5f, 0);
    cameraDir = glm::normalize(droneDir);
}

void drawDrone(void (*drawFunction)(Core::RenderContext&, glm::mat4, glm::vec3, float, float)) {
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

    for (auto& context : models::droneContexts) {
        drawFunction(context, modelMatrix, glm::vec3(0.8f), 0.2f, 0.2f);
    }
}