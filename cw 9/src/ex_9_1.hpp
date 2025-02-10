#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"

#include "Texture.h"
#include "boid.h"

#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include "Skybox.h"
#include "PerlinNoise.h"
#include "BoidInteraction.h"
#include "Drone.h"

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

int WIDTH = 500, HEIGHT = 500;

namespace models {
	Core::RenderContext paperplaneContext;
	Core::RenderContext templeContext;
	Core::RenderContext sphereContext;
	Core::RenderContext treeContext;
	Core::RenderContext leavesContext;
	Core::RenderContext testContext;
}

namespace texture {
	GLuint bark;
	GLuint leaves;
	GLuint temple;
}

GLuint depthMapFBO;
GLuint depthMap;

GLuint program;
GLuint programTest;
GLuint programTex;
GLuint programTextured;
GLuint programDepth;

GLuint terrainTexture;
GLuint terrainNormalMap;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;

//glm::vec3 sunPos = glm::vec3(-4.740971f, 2.149999f, 0.369280f);
//glm::vec3 sunPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 sunPos = glm::vec3(10.0f, 40.0f, -65.0f);

//glm::vec3 sunDir = glm::normalize(glm::vec3(-0.228586f, -0.584819f, 0.778293f)); // Kierunek słońca dopasowany do SkyBox
glm::vec3 sunDir = glm::normalize(glm::vec3(0.228586f, 0.584819f, -0.778293f));
glm::vec3 sunColor = glm::vec3(0.8f, 0.8f, 0.6f) * 3.0f; // Bright sunlight



float aspectRatio = 1.f;
float exposition = 1.f;

glm::vec3 spotlightPos = glm::vec3(0, 0, 0);
glm::vec3 spotlightConeDir = glm::vec3(0, 0, 0);
glm::vec3 spotlightColor = glm::vec3(0.4, 0.4, 0.9) * 3;
float spotlightPhi = 3.14 / 4;

float lastTime = -1.f;
float deltaTime = 0.f;

std::vector<glm::vec3> treePositions; // Global variable to store tree positions

// Define the flat area for the temple
const float flatAreaSize = 20.0f; // Size of the flat area
const float flatAreaHeight = 0.0f; // Height of the flat area

float near_plane = 0.05f, far_plane = 200.0f;
glm::mat4 templeModelMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, flatAreaHeight, 0.0f)) * glm::scale(glm::mat4(), glm::vec3(0.5));


// Terrain generation and rendering functions
void generateTerrain();
void drawTerrain(glm::mat4 viewProjectionMatrix);
void initTerrainShader();

// Terrain variables
const int terrainSize = 350;
const float terrainScale = 1.0f;
GLuint terrainVAO, terrainVBO, terrainEBO;
GLuint terrainShader;



// Global Perlin noise object
PerlinNoise perlinNoise;

float generateHeight(float x, float z) {
	// Check if the current position is within the flat area
	if (x >= -flatAreaSize / 2 && x <= flatAreaSize / 2 &&
		z >= -flatAreaSize / 2 && z <= flatAreaSize / 2) {
		return flatAreaHeight; // Return the flat height
	}

	// Otherwise, generate terrain height using Perlin noise
	float scale = 0.065f;
	float heightScale = 5.0f;
	float baseHeight = -2.6f; // Lower the terrain by 5 units
	return perlinNoise.noise(x * scale, z * scale, 0.0f) * heightScale + baseHeight;
}

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

void drawTerrain(glm::mat4 viewProjectionMatrix) {
	glUseProgram(terrainShader);

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


void initTerrainShader() {
	terrainShader = shaderLoader.CreateProgram("shaders/terrain.vert", "shaders/terrain.frag");
}



void updateDeltaTime(float time) {
	if (lastTime < 0) {
		lastTime = time;
		return;
	}

	deltaTime = time - lastTime;
	if (deltaTime > 0.1) deltaTime = 0.1;
	lastTime = time;
}

glm::mat4 createCameraMatrix() {
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide, cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x, cameraSide.y, cameraSide.z, 0,
		cameraUp.x, cameraUp.y, cameraUp.z, 0,
		-cameraDir.x, -cameraDir.y, -cameraDir.z, 0,
		0., 0., 0., 1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}

glm::mat4 createPerspectiveMatrix() {
	glm::mat4 perspectiveMatrix;
	float n = near_plane;  // Near plane
	float f = far_plane; // Far plane (increased from 20.0 to 200.0)
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1, 0., 0., 0.,
		0., aspectRatio, 0., 0.,
		0., 0., (f + n) / (n - f), 2 * f * n / (n - f),
		0., 0., -1., 0.,
		});

	perspectiveMatrix = glm::transpose(perspectiveMatrix);
	return perspectiveMatrix;
}

// TODO : Zintegorwać drawPBR by używało tekstur 
void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, float roughness, float metallic) {
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);

	glUniform1f(glGetUniformLocation(program, "roughness"), roughness);
	glUniform1f(glGetUniformLocation(program, "metallic"), metallic);

	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);

	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniform3f(glGetUniformLocation(program, "sunDir"), sunDir.x, sunDir.y, sunDir.z);
	glUniform3f(glGetUniformLocation(program, "sunColor"), sunColor.x, sunColor.y, sunColor.z);

	glUniform3f(glGetUniformLocation(program, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(program, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(program, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(program, "spotlightPhi"), spotlightPhi);
	Core::DrawContext(context);
}

void drawObjectTextured(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID) {
	glUseProgram(programTextured);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(programTextured, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programTextured, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);


	// Przekaż teksturę depthMap
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(glGetUniformLocation(programTextured, "depthMap"), 0); // Przekaż indeks tekstury (0 dla GL_TEXTURE0)

	// Przekaż macierz LightVP

	glm::mat4 lightVP = glm::ortho(-50.f, 50.f, -50.f, 50.f, near_plane, far_plane) * glm::lookAt(sunPos, sunPos - sunDir, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(programTextured, "LightVP"), 1, GL_FALSE, (float*)&lightVP);

	glUniform3f(glGetUniformLocation(programTextured, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(programTextured, "sunDir"), sunDir.x, sunDir.y, sunDir.z);
	glUniform3f(glGetUniformLocation(programTextured, "sunColor"), sunColor.x, sunColor.y, sunColor.z);
	glUniform3f(glGetUniformLocation(programTextured, "sunPos"), sunPos.x, sunPos.y, sunPos.z);

	Core::SetActiveTexture(textureID, "colorTexture", programTextured, 0);
	glUniform1i(glGetUniformLocation(programTextured, "colorTexture"), 0);

	Core::DrawContext(context);
	glUseProgram(0);
}

void initShadowMap() {
	// Create the depth map texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Create the FBO and attach the depth texture
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawObjectDepth(const Core::RenderContext& context, const glm::mat4& viewProjection, const glm::mat4& model)
{
	GLuint viewProjectionLoc = glGetUniformLocation(programDepth, "viewProjectionMatrix");
	GLuint modelLoc = glGetUniformLocation(programDepth, "modelMatrix");

	// Ustawienie macierzy w shaderze
	glUniformMatrix4fv(viewProjectionLoc, 1, GL_FALSE, glm::value_ptr(viewProjection));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// Przypisanie VAO
	glBindVertexArray(context.vertexArray);

	// Rysowanie obiektu
	glDrawElements(GL_TRIANGLES, context.size, GL_UNSIGNED_INT, 0);

	// Odwiązanie VAO
	glBindVertexArray(0);

}
void renderShadowapSun()
{
	float time = glfwGetTime();
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);


	glm::mat4 lightVP = glm::ortho(-50.f, 50.f, -50.f, 50.f, near_plane, far_plane) * glm::lookAt(sunPos, sunPos - sunDir, glm::vec3(0, 1, 0));

	glUseProgram(programDepth);
	GLuint lightSpaceMatrixLoc = glGetUniformLocation(programDepth, "lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightVP));


	drawObjectDepth(models::templeContext, lightVP, templeModelMatrix);
	for (const auto& position : treePositions) {
		// Create model matrix for the tree
		glm::mat4 treeModelMatrix = glm::translate(glm::mat4(), position) * glm::scale(glm::mat4(), glm::vec3(0.2));
		drawObjectDepth(models::treeContext, lightVP, treeModelMatrix);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// Globalna zmienna przechowująca boidy
std::vector<Boid> boids;

void renderScene(GLFWwindow* window) {
	glClearColor(0.537f, 0.812f, 0.941f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderShadowapSun();

	drawSkybox(createCameraMatrix(), createPerspectiveMatrix());

	float time = glfwGetTime();
	updateDeltaTime(time);

	// Render terrain
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glUseProgram(terrainShader);
	glUniformMatrix4fv(glGetUniformLocation(terrainShader, "mvp"), 1, GL_FALSE, &(viewProjectionMatrix * modelMatrix)[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(terrainShader, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
	glUniform3f(glGetUniformLocation(terrainShader, "sunDir"), sunDir.x, sunDir.y, sunDir.z);
	glUniform3f(glGetUniformLocation(terrainShader, "sunColor"), sunColor.x, sunColor.y, sunColor.z);
	glUniform3f(glGetUniformLocation(terrainShader, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	drawTerrain(viewProjectionMatrix);

	glUseProgram(program);
	drawDrone(drawObjectPBR);

	// Update boids
	for (auto& boid : boids) {
		boid.update(boids);


	}

	for (const auto& boid : boids) {
		float horizontalAngle = boid.getHorizontalAngle();
		float verticalAngle = boid.getVerticalAngle();

		// Tworzenie macierzy transformacji
		glm::mat4 modelMatrix = glm::translate(glm::mat4(), boid.getPosition()) *
			glm::rotate(glm::mat4(1.0f), horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), verticalAngle, glm::vec3(1.0f, 0.0f, 0.0f)) *   
			glm::scale(glm::vec3(0.005f)); 

		drawObjectPBR(models::paperplaneContext, modelMatrix, glm::vec3(boid.getGroupId() * 40.0f, 10.0f, boid.getGroupId()*40.0f), 0.2f, 0.0f);
	}

	spotlightPos = dronePos + 0.2 * droneDir;
	spotlightConeDir = droneDir;

	renderInteractionIndicators(drawObjectPBR);
  
	glUseProgram(programTextured);
	for (const auto& position : treePositions) {
		// Create model matrix for the tree
		glm::mat4 treeModelMatrix = glm::translate(glm::mat4(), position) * glm::scale(glm::mat4(), glm::vec3(0.2));

		// Draw the tree trunk
		drawObjectTextured(models::treeContext, treeModelMatrix, texture::bark);

		// Draw the tree leaves
		drawObjectTextured(models::leavesContext, treeModelMatrix, texture::leaves);
	}


	
	drawObjectTextured(models::templeContext, templeModelMatrix, texture::temple);

	//test depth buffer
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glUseProgram(programTest);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, depthMap);
	//Core::DrawContext(models::testContext);



	glUseProgram(0);
	glfwSwapBuffers(window);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
	WIDTH = width;
	HEIGHT = height;
}
void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void init(GLFWwindow* window)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	program = shaderLoader.CreateProgram("shaders/shader_9_1.vert", "shaders/shader_9_1.frag");
	programTest = shaderLoader.CreateProgram("shaders/test.vert", "shaders/test.frag");
	programTextured = shaderLoader.CreateProgram("shaders/shader_textured.vert", "shaders/shader_textured.frag");
	programDepth = shaderLoader.CreateProgram("shaders/shadow.vert", "shaders/shadow.frag");
	for (int groupId = 0; groupId < 6; ++groupId) {
		for (int i = 0; i < 40; ++i) {
			glm::vec3 position(
				(rand() % 200 + 0.1f) / 100.f,
				(rand() % 200 + 0.1f) / 100.f,
				(rand() % 200 + 0.1f)/100.f



			);
			boids.push_back(Boid(position, groupId,i));
		}
	}

	for (int i = 0; i < 100; ++i) {
		// Generate random position within the terrain bounds
		float x = static_cast<float>(rand() % terrainSize) - terrainSize / 2;
		float z = static_cast<float>(rand() % terrainSize) - terrainSize / 2;
		float y = generateHeight(x, z); // Get the height of the terrain at this position

		// Store the position in the global vector
		treePositions.push_back(glm::vec3(x, y, z));
	}

	initSkybox(shaderLoader, loadModelToContext);
	loadDroneModel("./models/drone.obj");
	
	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship.obj", shipContext);

	loadModelToContext("./models/paperAirplane.obj", models::paperplaneContext);
	loadModelToContext("./models/temple.obj", models::templeContext);

	loadModelToContext("./models/MapleTree.obj", models::treeContext);
	loadModelToContext("./models/MapleTreeLeaves.obj", models::leavesContext);

	loadModelToContext("./models/test.obj", models::testContext);


	// Load terrain texture
	terrainTexture = Core::LoadTexture("./textures/terrain/terrain.jpg");
	terrainNormalMap = Core::LoadTexture("./textures/terrain/terrain_normal.jpg");
	if (terrainTexture == 0 || terrainNormalMap == 0) {
		std::cerr << "Failed to load terrain texture!" << std::endl;
	}
	// Initialize terrain
	generateTerrain();
	initTerrainShader();

	texture::bark = Core::LoadTexture("textures/bark.jpg");
	texture::leaves = Core::LoadTexture("textures/leaf.png");
	texture::temple = Core::LoadTexture("textures/temple/map.png");

	initShadowMap();
}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) exposition -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) exposition += 0.05;

	handleBoidInteraction(window, boids);

	updateDrone(window, deltaTime);
	updateCamera();
}

// funkcja jest glowna petla
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderScene(window);
		glfwPollEvents();
	}
}