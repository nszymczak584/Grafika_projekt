#include "Temple.h"
#include "Shader_Loader.h"
#include "Texture.h"
#include <gtc/matrix_transform.hpp>
#include <iostream>

// Kontekst renderowania œwi¹tyni
Core::RenderContext templeContext;

// Tekstury œwi¹tyni
GLuint templeTexture, templeNormalMap;
extern const float flatAreaHeight;
extern void loadModelToContext(std::string path, Core::RenderContext& context);
extern void drawObjectTexturedNormal(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapID, float ambientLight = 0.2f);

// Wczytywanie modelu œwi¹tyni i tekstur
void initTemple() {
    // Za³aduj model œwi¹tyni
    loadModelToContext("./models/temple.obj", templeContext);

    // Za³aduj tekstury œwi¹tyni
    templeTexture = Core::LoadTexture("textures/temple/temple.png");
    templeNormalMap = Core::LoadTexture("textures/temple/temple_normal.png");
}

// Rysowanie œwi¹tyni
void drawTemple() {
    // Macierz modelu dla œwi¹tyni
    glm::mat4 templeModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, flatAreaHeight, 0.0f))
        * glm::rotate(glm::mat4(1.0f), glm::radians(140.0f), glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(0.5));

    // Renderowanie modelu œwi¹tyni
    drawObjectTexturedNormal(templeContext, templeModelMatrix, templeTexture, templeNormalMap);
}
