#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include "collision.h"
#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"
#include "boid.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include "Skybox.h"
#include "PerlinNoise.h"
#include "BoidInteraction.h"
#include "Drone.h"

int WIDTH, HEIGHT;

namespace texture {
	GLuint vase;
	GLuint pillar;
	GLuint tent;
	GLuint paper;
	GLuint paper2;
	GLuint paper3;
	GLuint paper4;
	GLuint paper5;
}

GLuint program;
GLuint programLines;
GLuint programTexturedNormal;

ModelData airplaneData;
ModelData benchData;
ModelData vaseData;
std::vector<CollidableObject> collidableObjects; // Globalna lista przeszkód

std::vector<Boid> boids;
namespace models {
	Core::RenderContext paperplaneContext;
	Core::RenderContext benchContext;
	Core::RenderContext vaseContext;
	Core::RenderContext tentContext;
}

glm::vec3 sunDir = glm::normalize(glm::vec3(0.228586f, 0.584819f, -0.778293f));
glm::vec3 sunColor = glm::vec3(0.8f, 0.8f, 0.6f) * 4.0f;
float exposition = 2.f;

void drawBoundingBox(const BoundingBox& bbox, const glm::vec3& color) {
	glUseProgram(programLines);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 modelMatrix = glm::mat4(1.0f); 
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(programLines, "transformation"), 1, GL_FALSE, &transformation[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programLines, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
	glUniform3f(glGetUniformLocation(programLines, "color"), color.x, color.y, color.z);

	// Wierzchołki bounding boxa
	glm::vec3 vertices[] = {
		{bbox.min.x, bbox.min.y, bbox.min.z}, // 0
		{bbox.max.x, bbox.min.y, bbox.min.z}, // 1
		{bbox.max.x, bbox.min.y, bbox.max.z}, // 2
		{bbox.min.x, bbox.min.y, bbox.max.z}, // 3

		{bbox.min.x, bbox.max.y, bbox.min.z}, // 4
		{bbox.max.x, bbox.max.y, bbox.min.z}, // 5
		{bbox.max.x, bbox.max.y, bbox.max.z}, // 6
		{bbox.min.x, bbox.max.y, bbox.max.z}  // 7
	};

	// Indeksy do rysowania linii
	unsigned int indices[] = {
		0, 1, 1, 2, 2, 3, 3, 0, // Dolna podstawa
		4, 5, 5, 6, 6, 7, 7, 4, // Górna podstawa
		0, 4, 1, 5, 2, 6, 3, 7  // Połączenia między podstawami
	};

	GLuint vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	// Bind vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Bind indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Set vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Draw lines
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Cleanup
	glBindVertexArray(0);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);

	glUseProgram(0);
}
void drawCubeFrames(const glm::mat4& modelMatrix, const glm::vec3& color) {
	glUseProgram(programLines);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, &transformation[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);

	glm::vec3 vertices[] = {
		{-5.0f, 1.45f, -4.0f}, {4.5f, 1.45f, -4.0f},   // Dolna podstawa (y = 1)
		{4.5f, 1.45f, 2.0f},  {-5.0f, 1.45f, 2.0f},

		{-5.0f, 6.0f, -4.0f}, {4.5f, 6.0f, -4.0f},   // Górna podstawa (y = 6)
		{4.5f, 6.0f, 2.0f},  {-5.0f, 6.0f, 2.0f}
	};



	unsigned int indices[] = {
		0, 1, 1, 2, 2, 3, 3, 0, 
		4, 5, 5, 6, 6, 7, 7, 4, 
		0, 4, 1, 5, 2, 6, 3, 7  
	};

	GLuint vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(0);
	glUseProgram(0);

	// Cleanup
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}

void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, float roughness, float metallic) {
	glUseProgram(program);
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

	Core::DrawContext(context);
	glUseProgram(0);
}

void drawObjectTexturedNormal(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapID, float ambientLight = 0.2f) {
	glUseProgram(programTexturedNormal);

	GLuint selectedNormalMap = useNormalMapping ? normalMapID : flatNormalMap;

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programTexturedNormal, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programTexturedNormal, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform3f(glGetUniformLocation(programTexturedNormal, "sunDir"), sunDir.x, sunDir.y, sunDir.z);

	glUniform1f(glGetUniformLocation(programTexturedNormal, "ambientLight"), ambientLight);

	Core::SetActiveTexture(textureID, "colorTexture", programTexturedNormal, 0);
	Core::SetActiveTexture(selectedNormalMap, "normalMap", programTexturedNormal, 1);
	glUniform1i(glGetUniformLocation(programTexturedNormal, "colorTexture"), 0);
	glUniform1i(glGetUniformLocation(programTexturedNormal, "normalMap"), 1);

	Core::DrawContext(context);
	glUseProgram(0);
}

void renderScene(GLFWwindow* window) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawSkybox();
	drawDrone();
	drawTrees();
	drawInteractionSpheres();
	drawTemple();
	drawTerrain();
	//drwaBoids();

	// ------- Boids --------
	for (auto& boid : boids) boid.update(boids, collidableObjects);

	for (auto& boid : boids) {
		float horizontalAngle = boid.getHorizontalAngle();
		float verticalAngle = boid.getVerticalAngle();
		glm::mat4 boidmodelMatrix = glm::translate(glm::mat4(), boid.getPosition()) *
			glm::rotate(glm::mat4(1.0f), horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), verticalAngle, glm::vec3(1.0f, 0.0f, 0.0f)) *   
			glm::scale(glm::vec3(0.01f)); 
		BoundingBox bBox = calculateBoundingBox(airplaneData.localBBox, boidmodelMatrix);
		boid.setBoundingBox(bBox);
		GLuint boidTexture;
		switch (boid.getGroupId()) {
		case 0: boidTexture = texture::paper; break;
		case 1: boidTexture = texture::paper2; break;
		case 2: boidTexture = texture::paper3; break;
		case 3: boidTexture = texture::paper4; break;
		case 4: boidTexture = texture::paper5; break;
		default: boidTexture = texture::paper; break;
		}
		drawObjectTextured(models::paperplaneContext, boidmodelMatrix, boidTexture);
		
	}
	/*for (auto& boid : boids) {
		float horizontalAngle = boid.getHorizontalAngle();
		float verticalAngle = boid.getVerticalAngle();
		glm::mat4 boidmodelMatrix = glm::translate(glm::mat4(), boid.getPosition()) *
			glm::rotate(glm::mat4(1.0f), horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), verticalAngle, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::vec3(0.01f));
		drawBoundingBox(calculateBoundingBox(airplaneData.localBBox, boidmodelMatrix), glm::vec3(1.0f, 0.0f, 0.0f));
	}*/
	glm::mat4 cubeModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); 
	/*drawCubeFrames(cubeModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));*/

	// Tent
	glm::mat4 tentModelMatrix = glm::translate(glm::mat4(), glm::vec3(-30.0f, 0.f, 12.0f)) * 
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::scale(glm::mat4(), glm::vec3(2.0f));
	drawObjectTextured(models::tentContext, tentModelMatrix, texture::tent);

	// Vase
	glm::mat4 vaseModelMatrix = glm::translate(glm::mat4(), glm::vec3(-2.0f, flatAreaHeight + 0.8f, 1.5f)) * glm::scale(glm::mat4(), glm::vec3(0.001));
	drawObjectTextured(models::vaseContext, vaseModelMatrix, texture::vase);

	// Bench
	float benchSpacing = 2.0f; 
	for (int i = 0; i < 2; ++i) {
		glm::vec3 benchPosition = glm::vec3(0.5f + i * benchSpacing, flatAreaHeight + 0.9f, -1.0f);
		glm::mat4 benchModelMatrix = glm::translate(glm::mat4(), benchPosition) * glm::scale(glm::mat4(), glm::vec3(0.75f));
		drawObjectTextured(models::benchContext, benchModelMatrix, texture::pillar);
		/*drawBoundingBox(calculateBoundingBox(benchData.localBBox, benchModelMatrix), glm::vec3(1.0f, 0.0f, 0.0f));*/
	}

	glfwSwapBuffers(window);
}
void init(GLFWwindow* window) {
	glfwSetScrollCallback(window, scroll_callback);
	glEnable(GL_DEPTH_TEST);

	initSkybox();
	initDrone();
	initTrees();
	initInteractionSpheres();
	initTemple();
	initTerrain();
	//initBoids();

	initNormalMapToggle();

	// **Inicjalizacja shaderów**
	program = Core::Shader_Loader().CreateProgram("shaders/shader_pbr.vert", "shaders/shader_pbr.frag");
	programLines = Core::Shader_Loader().CreateProgram("shaders/shader_lines.vert", "shaders/shader_lines.frag");
	programTexturedNormal = Core::Shader_Loader().CreateProgram("shaders/shader_textured_normal.vert", "shaders/shader_textured_normal.frag");
	
	// **Generowanie boidów**
	for (int groupId = 0; groupId < 5; ++groupId) {
		for (int i = 0; i < 20; ++i) {
			glm::vec3 position(
				(rand() % 850 -400.f)/100.0f,  // Losowanie x w zakresie [-4.0f, 4.5f]
				(rand() % 455 + 145.f)/100.0f,          // Losowanie y w zakresie [1.45f, 6.0f]
				(rand() % 600-400.f)/100.0f  // Losowanie z w zakresie [-4.0f, 2.0f]
			);
			boids.push_back(Boid(position, groupId, i));
		}
	}
	loadModelToContext("./models/PaperAirplane.obj", models::paperplaneContext);

	loadModelToContext("./models/tent.obj", models::tentContext);
	loadModelToContext("./models/objBench.obj", models::benchContext);
	loadModelToContext("./models/Vase.obj", models::vaseContext);
	airplaneData = loadModel("./models/PaperAirplane.obj");
	benchData = loadModel("./models/objBench.obj");
	vaseData = loadModel("./models/Vase.obj");
	glm::mat4 vaseModelMatrix = glm::translate(glm::mat4(), glm::vec3(-2.0f, flatAreaHeight + 0.8f, 1.5f)) * glm::scale(glm::mat4(), glm::vec3(0.001));
	BoundingBox vaseBBox = calculateBoundingBox(vaseData.localBBox, vaseModelMatrix);
	collidableObjects.push_back({ vaseBBox });
	float benchSpacing = 2.0f;
	for (int i = 0; i < 2; ++i) {
		glm::vec3 benchPosition = glm::vec3(0.5f + i * benchSpacing, flatAreaHeight + 0.9f, -1.0f);
		glm::mat4 benchModelMatrix = glm::translate(glm::mat4(), benchPosition) * glm::scale(glm::mat4(), glm::vec3(0.75f));
		BoundingBox benchBBox = calculateBoundingBox(benchData.localBBox, benchModelMatrix);

		collidableObjects.push_back({ benchBBox });
	}
	texture::tent = Core::LoadTexture("textures/tent/dome_tent_BaseColor.png");
	texture::vase = Core::LoadTexture("textures/vase/vase.jpg");
	texture::pillar = Core::LoadTexture("textures/pillar/concrete_0018_color_1k.jpg");
	texture::paper = Core::LoadTexture("textures/paper/paper_0022_color_1k.jpg");
	texture::paper2 = Core::LoadTexture("textures/paper/paper_0022_color_1k_2.png");
	texture::paper3 = Core::LoadTexture("textures/paper/paper_0022_color_1k_3.png");
	texture::paper4 = Core::LoadTexture("textures/paper/paper_0022_color_1k_4.png");
	texture::paper5 = Core::LoadTexture("textures/paper/paper_0022_color_1k_5.png");
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) exposition -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) exposition += 0.05;

	handleBoidInteraction(window, boids, collidableObjects); // Nowe poprawne
	handleNormalMapToggle(window);
	updateDrone(window);
	updateCamera();
}

void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		renderScene(window);
		glfwPollEvents();
	}
}

void shutdown(GLFWwindow* window) {
	Core::Shader_Loader().DeleteProgram(program);
}