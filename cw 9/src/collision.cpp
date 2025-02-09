// DOP14.cpp
#include "collision.h"
#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <assimp/scene.h>
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <limits>



ModelData loadModel(const std::string& path) {
    ModelData modelData;

    Assimp::Importer import;
    const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "B³¹d ³adowania modelu: " << import.GetErrorString() << std::endl;
        return modelData; 
    }

    modelData.mesh = scene->mMeshes[0];


    if (modelData.mesh) { 
        modelData.localBBox.min = glm::vec3(std::numeric_limits<float>::max());
        modelData.localBBox.max = glm::vec3(std::numeric_limits<float>::lowest());

        for (unsigned int i = 0; i < modelData.mesh->mNumVertices; ++i) {
            aiVector3D vertex = modelData.mesh->mVertices[i];
            modelData.localBBox.min.x = std::min(modelData.localBBox.min.x, vertex.x);
            modelData.localBBox.min.y = std::min(modelData.localBBox.min.y, vertex.y);
            modelData.localBBox.min.z = std::min(modelData.localBBox.min.z, vertex.z);

            modelData.localBBox.max.x = std::max(modelData.localBBox.max.x, vertex.x);
            modelData.localBBox.max.y = std::max(modelData.localBBox.max.y, vertex.y);
            modelData.localBBox.max.z = std::max(modelData.localBBox.max.z, vertex.z);
        }
    }

    return modelData;
}

BoundingBox calculateBoundingBox(const BoundingBox& localBBox, const glm::mat4& transformationMatrix) {
    glm::vec4 vertices[8] = {
        glm::vec4(localBBox.min.x, localBBox.min.y, localBBox.min.z, 1.0f),
        glm::vec4(localBBox.max.x, localBBox.min.y, localBBox.min.z, 1.0f),
        glm::vec4(localBBox.max.x, localBBox.max.y, localBBox.min.z, 1.0f),
        glm::vec4(localBBox.min.x, localBBox.max.y, localBBox.min.z, 1.0f),
        glm::vec4(localBBox.min.x, localBBox.min.y, localBBox.max.z, 1.0f),
        glm::vec4(localBBox.max.x, localBBox.min.y, localBBox.max.z, 1.0f),
        glm::vec4(localBBox.max.x, localBBox.max.y, localBBox.max.z, 1.0f),
        glm::vec4(localBBox.min.x, localBBox.max.y, localBBox.max.z, 1.0f)
    };
    BoundingBox transformedBBox;
    transformedBBox.min = glm::vec3(std::numeric_limits<float>::max());
    transformedBBox.max = glm::vec3(std::numeric_limits<float>::lowest());
    for (int i = 0; i < 8; ++i) {
        glm::vec4 transformedVertex4 = transformationMatrix * vertices[i];
        glm::vec3 transformedVertex = glm::vec3(transformedVertex4);

        transformedBBox.min.x = std::min(transformedBBox.min.x, transformedVertex.x);
        transformedBBox.min.y = std::min(transformedBBox.min.y, transformedVertex.y);
        transformedBBox.min.z = std::min(transformedBBox.min.z, transformedVertex.z);

        transformedBBox.max.x = std::max(transformedBBox.max.x, transformedVertex.x);
        transformedBBox.max.y = std::max(transformedBBox.max.y, transformedVertex.y);
        transformedBBox.max.z = std::max(transformedBBox.max.z, transformedVertex.z);
    }
    return transformedBBox;
}