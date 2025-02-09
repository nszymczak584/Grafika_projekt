
#pragma once
#include "glm.hpp"
#include <array>
#include <assimp/scene.h>


struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
};
struct ModelData {
    const aiMesh* mesh = nullptr;
    BoundingBox localBBox;
};
struct CollidableObject {
    BoundingBox bbox;
};
ModelData loadModel(const std::string& path);
BoundingBox calculateBoundingBox(const BoundingBox& localBBox, const glm::mat4& transformationMatrix);