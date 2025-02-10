#include "Terrain.h"
#include "Shader_Loader.h"
#include "Texture.h"
#include "PerlinNoise.h"
#include <vector>
#include <gtc/matrix_transform.hpp>

// Definicje globalnych zmiennych
const float flatAreaSize = 20.0f;
const float flatAreaHeight = 0.0f;
const int terrainSize = 350;
const float terrainScale = 1.0f;

GLuint terrainVAO, terrainVBO, terrainEBO;
GLuint terrainShader;
PerlinNoise perlinNoise;

extern glm::vec3 sunDir;
extern glm::vec3 sunColor;
extern glm::vec3 cameraPos;

// Tekstury terenu
GLuint terrainTexture, terrainNormalMap;

extern glm::mat4 createPerspectiveMatrix();
extern glm::mat4 createCameraMatrix();

// Funkcja generuj¹ca wysokoœæ terenu z u¿yciem Perlin Noise
float generateHeight(float x, float z) {
    if (x >= -flatAreaSize / 2 && x <= flatAreaSize / 2 &&
        z >= -flatAreaSize / 2 && z <= flatAreaSize / 2) {
        return flatAreaHeight;  // P³aska powierzchnia na œrodku
    }

    float scale = 0.065f;
    float heightScale = 5.0f;
    float baseHeight = -2.6f;
    return perlinNoise.noise(x * scale, z * scale, 0.0f) * heightScale + baseHeight;
}

// Generowanie siatki terenu
void generateTerrain() {
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> normals;
	std::vector<float> texCoords; // Texture coordinates

	// Generate vertices, normals, and texture coordinates
	for (int z = 0; z < terrainSize; ++z) {
		for (int x = 0; x < terrainSize; ++x) {
			float worldX = (x - terrainSize / 2) * terrainScale;
			float worldZ = (z - terrainSize / 2) * terrainScale;
			float height = generateHeight(worldX, worldZ);

			// Vertex position
			vertices.push_back(worldX);
			vertices.push_back(height);
			vertices.push_back(worldZ);

			// Texture coordinates (normalized to [0, 1])
			texCoords.push_back((float)x / (terrainSize - 1));
			texCoords.push_back((float)z / (terrainSize - 1));
		}
	}

	// Calculate normals (same as before)
	for (int z = 0; z < terrainSize; ++z) {
		for (int x = 0; x < terrainSize; ++x) {
			int index = z * terrainSize + x;

			// Get neighboring heights
			float hL = (x > 0) ? vertices[(z * terrainSize + (x - 1)) * 3 + 1] : vertices[index * 3 + 1];
			float hR = (x < terrainSize - 1) ? vertices[(z * terrainSize + (x + 1)) * 3 + 1] : vertices[index * 3 + 1];
			float hD = (z > 0) ? vertices[((z - 1) * terrainSize + x) * 3 + 1] : vertices[index * 3 + 1];
			float hU = (z < terrainSize - 1) ? vertices[((z + 1) * terrainSize + x) * 3 + 1] : vertices[index * 3 + 1];

			// Calculate normal
			glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 1.0f, hD - hU));
			normals.push_back(normal.x);
			normals.push_back(normal.y);
			normals.push_back(normal.z);
		}
	}

	// Generate indices (same as before)
	for (int z = 0; z < terrainSize - 1; ++z) {
		for (int x = 0; x < terrainSize - 1; ++x) {
			int start = z * terrainSize + x;
			indices.push_back(start);
			indices.push_back(start + terrainSize);
			indices.push_back(start + 1);

			indices.push_back(start + 1);
			indices.push_back(start + terrainSize);
			indices.push_back(start + terrainSize + 1);
		}
	}

	// Create VAO, VBO, and EBO
	glGenVertexArrays(1, &terrainVAO);
	glGenBuffers(1, &terrainVBO);
	glGenBuffers(1, &terrainEBO);

	glBindVertexArray(terrainVAO);

	// Bind vertices, normals, and texture coordinates to a single VBO
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glBufferData(GL_ARRAY_BUFFER, (vertices.size() + normals.size() + texCoords.size()) * sizeof(float), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), normals.size() * sizeof(float), normals.data());
	glBufferSubData(GL_ARRAY_BUFFER, (vertices.size() + normals.size()) * sizeof(float), texCoords.size() * sizeof(float), texCoords.data());

	// Bind indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// Set vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // Vertices
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(vertices.size() * sizeof(float))); // Normals
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)((vertices.size() + normals.size()) * sizeof(float))); // Texture coordinates
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

// Rysowanie terenu
void drawTerrain( GLuint depthMap, glm::vec3 sunPos, float near_plane, float far_plane) {
	glUseProgram(terrainShader);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	// Bind the terrain texture
	Core::SetActiveTexture(terrainTexture, "terrainTexture", terrainShader, 0);
	glUniform1i(glGetUniformLocation(terrainShader, "terrainTexture"), 0);

	// Bind the shadow map
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(glGetUniformLocation(terrainShader, "depthMap"), 1);

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 mvp = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(terrainShader, "mvp"), 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(terrainShader, "model"), 1, GL_FALSE, &modelMatrix[0][0]);

	// Pass LightVP matrix for shadow mapping
	glm::mat4 lightVP = glm::ortho(-50.f, 50.f, -50.f, 50.f, near_plane, far_plane) *
		glm::lookAt(sunPos, sunPos - sunDir, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(terrainShader, "LightVP"), 1, GL_FALSE, &lightVP[0][0]);

	// Set the texture scaling factor
	float textureScale = 10.0f;
	glUniform1f(glGetUniformLocation(terrainShader, "textureScale"), textureScale);

	// Lighting uniforms
	glUniform3f(glGetUniformLocation(terrainShader, "sunDir"), sunDir.x, sunDir.y, sunDir.z);
	glUniform3f(glGetUniformLocation(terrainShader, "sunColor"), sunColor.x, sunColor.y, sunColor.z);
	glUniform3f(glGetUniformLocation(terrainShader, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glBindVertexArray(terrainVAO);
	glDrawElements(GL_TRIANGLES, (terrainSize - 1) * (terrainSize - 1) * 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

// Inicjalizacja shaderów i tekstur terenu
void initTerrain() {
    terrainShader = Core::Shader_Loader().CreateProgram("shaders/shader_terrain.vert", "shaders/shader_terrain.frag");
    terrainTexture = Core::LoadTexture("./textures/terrain/terrain.jpg");
    terrainNormalMap = Core::LoadTexture("./textures/terrain/terrain_normal.jpg");

    generateTerrain();
}
