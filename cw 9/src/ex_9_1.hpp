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

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

int WIDTH = 500, HEIGHT = 500;

namespace models {
	Core::RenderContext paperplaneContext;
	Core::RenderContext bedContext;
	Core::RenderContext chairContext;
	Core::RenderContext deskContext;
	Core::RenderContext doorContext;
	Core::RenderContext drawerContext;
	Core::RenderContext marbleBustContext;
	Core::RenderContext materaceContext;
	Core::RenderContext pencilsContext;
	Core::RenderContext planeContext;
	Core::RenderContext roomContext;
	Core::RenderContext spaceshipContext;
	Core::RenderContext sphereContext;
	Core::RenderContext windowContext;
	Core::RenderContext testContext;
}

GLuint depthMapFBO;
GLuint depthMap;

GLuint program;
GLuint programSun;
GLuint programTest;
GLuint programTex;

GLuint terrainTexture;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;

//glm::vec3 sunPos = glm::vec3(-4.740971f, 2.149999f, 0.369280f);
//glm::vec3 sunPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 sunPos = glm::vec3(20.0f, 40.0f, -65.0f);

//glm::vec3 sunDir = glm::normalize(glm::vec3(-1.0f, -0.6f, 0.5f)); // Sun pointing downward
//glm::vec3 sunDir = glm::normalize(glm::vec3(-0.228586f, -0.584819f, 0.778293f)); // Kierunek słońca dopasowany do SkyBox
glm::vec3 sunDir = glm::normalize(glm::vec3(0.228586f, 0.584819f, -0.778293f));
glm::vec3 sunColor = glm::vec3(0.8f, 0.8f, 0.6f) * 4.0f; // Bright sunlight

glm::vec3 cameraPos = glm::vec3(0.479490f, 10.250000f, -20.124680f); // Adjust as needed
glm::vec3 cameraDir = glm::vec3(-0.354510f, 0.000000f, 0.935054f);

glm::vec3 spaceshipPos = glm::vec3(0.065808f, 1.250000f, -2.189549f);
//glm::vec3 spaceshipPos = sunPos;
glm::vec3 spaceshipDir = glm::vec3(-0.490263f, 0.000000f, 0.871578f);
//glm::vec3 spaceshipDir = sunDir;
GLuint VAO, VBO;

float aspectRatio = 1.f;

float exposition = 1.f;

glm::vec3 pointlightPos = glm::vec3(0, 2, 0);
glm::vec3 pointlightColor = glm::vec3(0.9, 0.6, 0.6);

glm::vec3 spotlightPos = glm::vec3(0, 0, 0);
glm::vec3 spotlightConeDir = glm::vec3(0, 0, 0);
glm::vec3 spotlightColor = glm::vec3(0.4, 0.4, 0.9) * 3;
float spotlightPhi = 3.14 / 4;

float lastTime = -1.f;
float deltaTime = 0.f;

// Terrain generation and rendering functions
void generateTerrain();
void drawTerrain(glm::mat4 viewProjectionMatrix);
void initTerrainShader();

// Terrain variables
const int terrainSize = 500;
const float terrainScale = 1.0f;
GLuint terrainVAO, terrainVBO, terrainEBO;
GLuint terrainShader;



// Global Perlin noise object
PerlinNoise perlinNoise;

float generateHeight(float x, float z) {
	float scale = 0.065f;
	float heightScale = 5.0f;
	float baseHeight = -4.0f; // Lower the terrain by 5 units
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

	// Bind the texture
	Core::SetActiveTexture(terrainTexture, "terrainTexture", terrainShader, 0);

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 mvp = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(terrainShader, "mvp"), 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(terrainShader, "model"), 1, GL_FALSE, &modelMatrix[0][0]);

	// Set the texture scaling factor
	float textureScale = 10.0f; // Adjust this value to control texture repetition
	glUniform1f(glGetUniformLocation(terrainShader, "textureScale"), textureScale);

	// Set lighting uniforms (if not already set)
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
	float n = 0.05;  // Near plane
	float f = 200.0; // Far plane (increased from 20.0 to 200.0)
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

	glUniform3f(glGetUniformLocation(program, "lightPos"), pointlightPos.x, pointlightPos.y, pointlightPos.z);
	glUniform3f(glGetUniformLocation(program, "lightColor"), pointlightColor.x, pointlightColor.y, pointlightColor.z);

	glUniform3f(glGetUniformLocation(program, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(program, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(program, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(program, "spotlightPhi"), spotlightPhi);
	Core::DrawContext(context);
}

void renderShadowapSun() {
	float time = glfwGetTime();
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	//uzupelnij o renderowanie glebokosci do tekstury

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WIDTH, HEIGHT);
}

// Globalna zmienna przechowująca boidy
std::vector<Boid> boids;

void renderScene(GLFWwindow* window) {
	glClearColor(0.537f, 0.812f, 0.941f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawSkybox(createCameraMatrix(), createPerspectiveMatrix());

	float time = glfwGetTime();
	updateDeltaTime(time);
	renderShadowapSun();

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
	// Clear the screen

	//space lamp
	glUseProgram(programSun);
	glm::mat4 transformation = viewProjectionMatrix * glm::translate(pointlightPos) * glm::scale(glm::vec3(0.1));
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform3f(glGetUniformLocation(programSun, "color"), sunColor.x / 2, sunColor.y / 2, sunColor.z / 2);
	glUniform1f(glGetUniformLocation(programSun, "exposition"), exposition);
	Core::DrawContext(sphereContext);

	glUseProgram(program);

	drawObjectPBR(sphereContext, glm::translate(pointlightPos) * glm::scale(glm::vec3(0.1)) * glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::scale(glm::vec3(0.3f)), glm::vec3(0.2, 0.7, 0.3), 0.3, 0.0);

	drawObjectPBR(sphereContext,
		glm::translate(pointlightPos) * glm::scale(glm::vec3(0.1)) * glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::eulerAngleY(time) * glm::translate(glm::vec3(1.f, 0, 0)) * glm::scale(glm::vec3(0.1f)),
		glm::vec3(0.5, 0.5, 0.5), 0.7, 0.0);

	drawObjectPBR(models::bedContext, glm::mat4(), glm::vec3(0.03f, 0.03f, 0.03f), 0.2f, 0.0f);
	drawObjectPBR(models::chairContext, glm::mat4(), glm::vec3(0.195239f, 0.37728f, 0.8f), 0.4f, 0.0f);
	drawObjectPBR(models::deskContext, glm::mat4(), glm::vec3(0.428691f, 0.08022f, 0.036889f), 0.2f, 0.0f);
	drawObjectPBR(models::doorContext, glm::mat4(), glm::vec3(0.402978f, 0.120509f, 0.057729f), 0.2f, 0.0f);
	drawObjectPBR(models::drawerContext, glm::mat4(), glm::vec3(0.428691f, 0.08022f, 0.036889f), 0.2f, 0.0f);
	drawObjectPBR(models::marbleBustContext, glm::mat4(), glm::vec3(1.f, 1.f, 1.f), 0.5f, 1.0f);
	drawObjectPBR(models::materaceContext, glm::mat4(), glm::vec3(0.9f, 0.9f, 0.9f), 0.8f, 0.0f);

	drawObjectPBR(models::planeContext, glm::mat4(), glm::vec3(0.402978f, 0.120509f, 0.057729f), 0.2f, 0.0f);
	drawObjectPBR(models::roomContext, glm::scale(glm::vec3(0.5)), glm::vec3(0.9f, 0.9f, 0.9f), 0.8f, 0.0f);
	drawObjectPBR(models::windowContext, glm::mat4(), glm::vec3(0.402978f, 0.120509f, 0.057729f), 0.2f, 0.0f);

	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});

	drawObjectPBR(shipContext,
		glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.03f)),
		glm::vec3(0.3, 0.3, 0.5),
		0.2, 1.0
	);
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

	spotlightPos = spaceshipPos + 0.2 * spaceshipDir;
	spotlightConeDir = spaceshipDir;

	renderInteractionIndicators(drawObjectPBR);

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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_9_1.vert", "shaders/shader_9_1.frag");
	programTest = shaderLoader.CreateProgram("shaders/test.vert", "shaders/test.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_8_sun.vert", "shaders/shader_8_sun.frag");
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

	initSkybox(shaderLoader, loadModelToContext);

	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship.obj", shipContext);
	loadModelToContext("./models/paperAirplane.obj", models::paperplaneContext);
	loadModelToContext("./models/bed.obj", models::bedContext);
	loadModelToContext("./models/chair.obj", models::chairContext);
	loadModelToContext("./models/desk.obj", models::deskContext);
	loadModelToContext("./models/door.obj", models::doorContext);
	loadModelToContext("./models/drawer.obj", models::drawerContext);
	loadModelToContext("./models/marbleBust.obj", models::marbleBustContext);
	loadModelToContext("./models/materace.obj", models::materaceContext);
	loadModelToContext("./models/pencils.obj", models::pencilsContext);
	loadModelToContext("./models/plane.obj", models::planeContext);
	loadModelToContext("./models/temple.obj", models::roomContext);
	loadModelToContext("./models/spaceship.obj", models::spaceshipContext);
	loadModelToContext("./models/sphere.obj", models::sphereContext);
	loadModelToContext("./models/window.obj", models::windowContext);
	loadModelToContext("./models/test.obj", models::testContext);


	// Load terrain texture
	terrainTexture = Core::LoadTexture("./textures/terrain.jpg");
	if (terrainTexture == 0) {
		std::cerr << "Failed to load terrain texture!" << std::endl;
	}
	// Initialize terrain
	generateTerrain();
	initTerrainShader();


}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
}

void processInput(GLFWwindow* window)
{
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f,1.f,0.f)));
	glm::vec3 spaceshipUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.05f * deltaTime * 60;
	float moveSpeed = 0.05f * deltaTime * 60;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		spaceshipPos += spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		spaceshipPos -= spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		spaceshipPos += spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		spaceshipPos -= spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		spaceshipPos += spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		spaceshipPos -= spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(angleSpeed) * glm::vec4(spaceshipDir, 0));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(-angleSpeed) * glm::vec4(spaceshipDir, 0));

	cameraPos = spaceshipPos - 0.5 * spaceshipDir + glm::vec3(0, 1, 0) * 0.2f;
	cameraDir = spaceshipDir;

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		exposition -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		exposition += 0.05;

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		printf("spaceshipPos = glm::vec3(%ff, %ff, %ff);\n", spaceshipPos.x, spaceshipPos.y, spaceshipPos.z);
		printf("spaceshipDir = glm::vec3(%ff, %ff, %ff);\n", spaceshipDir.x, spaceshipDir.y, spaceshipDir.z);
	}

	//cameraDir = glm::normalize(-cameraPos);


	// Spaceship Pitch
	// Pitch (obrót wokół osi bocznej statku)
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		glm::mat4 pitchRotation = glm::eulerAngleX(angleSpeed);
		spaceshipDir = glm::vec3(pitchRotation * glm::vec4(spaceshipDir, 0.0f));
		spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		glm::mat4 pitchRotation = glm::eulerAngleX(-angleSpeed);
		spaceshipDir = glm::vec3(pitchRotation * glm::vec4(spaceshipDir, 0.0f));
		spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	}

	printf("Wektor  : glm::vec3(%.6ff, %.6ff, %.6ff);\n",spaceshipDir.x, spaceshipDir.y, spaceshipDir.z);
	printf("Pozycja : glm::vec3(%.6ff, %.6ff, %.6ff);\n",spaceshipPos.x, spaceshipPos.y, spaceshipPos.z);

	handleBoidInteraction(window, boids);
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