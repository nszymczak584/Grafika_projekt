#include "Decorations.h"
#include "glew.h"
#include "Render_Utils.h"
#include "Shader_Loader.h"
#include "Texture.h"
#include <gtc/matrix_transform.hpp>

// Zewnêtrzne zmienne z g³ównego pliku
extern GLuint programTexturedNormal;
extern GLuint program;
extern const float flatAreaHeight;

// Struktura modeli dekoracji
namespace models {
    Core::RenderContext vaseContext;
    Core::RenderContext tentContext;
    Core::RenderContext benchContext;
}

// Struktura tekstur dekoracji
namespace texture {
    GLuint vase;
    GLuint tent;
    GLuint vaseNormalMap;
    GLuint tentNormalMap;
    GLuint pillar;
    GLuint concreteNormalMap;
}

extern void drawObjectTexturedNormal(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapID, float ambientLight = 0.2f);

// Funkcja inicjalizuj¹ca dekoracje
void initDecorations() {
    loadModelToContext("./models/tent.obj", models::tentContext);
    loadModelToContext("./models/Vase.obj", models::vaseContext);
    loadModelToContext("./models/objBench.obj", models::benchContext);

    texture::tent = Core::LoadTexture("textures/tent/tent.png");
    texture::tentNormalMap = Core::LoadTexture("textures/tent/tent_normal.png");
    texture::vase = Core::LoadTexture("textures/vase/vase.jpg");
    texture::vaseNormalMap = Core::LoadTexture("textures/vase/vase_normal.png");
    texture::pillar = Core::LoadTexture("textures/pillar/concrete.jpg");
    texture::concreteNormalMap = Core::LoadTexture("textures/pillar/concrete_normal.png");
}

// Funkcja rysuj¹ca dekoracje
void drawDecorations() {
    // Namiot
    glm::mat4 tentModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, 0.f, 12.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
    drawObjectTexturedNormal(models::tentContext, tentModelMatrix, texture::tent, texture::tentNormalMap);

    // Waza
    glm::mat4 vaseModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, flatAreaHeight + 0.8f, 1.5f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.001));
    drawObjectTexturedNormal(models::vaseContext, vaseModelMatrix, texture::vase, texture::vaseNormalMap);

    // Filary
    float benchSpacing = 2.0f;
    for (int i = 0; i < 2; ++i) {
        glm::vec3 benchPosition = glm::vec3(0.5f + i * benchSpacing, flatAreaHeight + 0.9f, -1.0f);
        glm::mat4 benchModelMatrix = glm::translate(glm::mat4(1.0f), benchPosition) * glm::scale(glm::mat4(), glm::vec3(0.75f));
        drawObjectTexturedNormal(models::benchContext, benchModelMatrix, texture::pillar, texture::concreteNormalMap);
    }
}