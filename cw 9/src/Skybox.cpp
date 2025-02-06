#include "skybox.h"
#include "SOIL/SOIL.h"
#include <iostream>

GLuint skyboxTexture;
GLuint skyboxShader;
Core::RenderContext skyBoxContext;

std::vector<std::string> skyboxFaces = {
    "./textures/skybox/sky_right.jpg",
    "./textures/skybox/sky_left.jpg",
    "./textures/skybox/sky_top.jpg",
    "./textures/skybox/sky_bottom.jpg",
    "./textures/skybox/sky_back.jpg",
    "./textures/skybox/sky_front.jpg",
};

GLuint loadCubemap(const std::vector<std::string>& faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (size_t i = 0; i < faces.size(); i++) {
        unsigned char* data = SOIL_load_image(faces[i].c_str(), &width, &height, &nrChannels, SOIL_LOAD_RGBA);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            std::cout << "Loaded: " << faces[i] << std::endl;
        }
        else {
            std::cerr << "Failed to load: " << faces[i] << std::endl;
        }
        SOIL_free_image_data(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void initSkybox(Core::Shader_Loader& shaderLoader, void(*loadModelFunc)(std::string, Core::RenderContext&)) {
    skyboxShader = shaderLoader.CreateProgram("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxTexture = loadCubemap(skyboxFaces);

    loadModelFunc("./models/cube.obj", skyBoxContext);
}

void drawSkybox(glm::mat4 cameraMatrix, glm::mat4 perspectiveMatrix) {
    glDepthMask(GL_FALSE);
    glUseProgram(skyboxShader);

    glm::mat4 viewProjectionMatrix = perspectiveMatrix * glm::mat4(glm::mat3(cameraMatrix));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "viewProjection"), 1, GL_FALSE, &viewProjectionMatrix[0][0]);

    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    Core::DrawContext(skyBoxContext);

    glDepthMask(GL_TRUE);
    glUseProgram(0);
}