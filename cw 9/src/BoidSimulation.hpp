#include "glew.h"
#include "Render_Utils.h"
#include "Shader_Loader.h"
#include "Texture.h"
#include <GLFW/glfw3.h>

#include "Boid.h"
#include "BoidInteraction.h"
#include "Collision.h"
#include "Drone.h"
#include "NormalMapToggle.h"
#include "Skybox.h"
#include "Temple.h"
#include "Terrain.h"
#include "Tree.h"
#include "Rocks.h"
#include "Decorations.h"

int WIDTH, HEIGHT;

namespace texture {
	GLuint paper;
	GLuint paper2;
	GLuint paper3;
	GLuint paper4;
	GLuint paper5;
	GLuint paperNormalMap;
}

GLuint program;
GLuint programLines;
GLuint programTexturedNormal;

ModelData airplaneData;
ModelData benchData;
ModelData vaseData;
std::vector<CollidableObject> collidableObjects;

std::vector<Boid> boids;
namespace models {
	Core::RenderContext paperplaneContext;
}

glm::vec3 sunDir = glm::normalize(glm::vec3(0.228586f, 0.584819f, -0.778293f));
glm::vec3 sunColor = glm::vec3(0.8f, 0.8f, 0.6f) * 4.0f;

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
void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, float roughness, float metallic, float exposition = 2.f) {
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

bool isBoidBoundingBoxVisible = false;
bool isCubeBoundingBoxVisible = false;
void toggleBoundingBoxBoids(GLFWwindow* window) {
	static bool bPressedLastFrame = false;
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
		if (!bPressedLastFrame) {
			isBoidBoundingBoxVisible = !isBoidBoundingBoxVisible;
		}
		bPressedLastFrame = true;
	}
	else {
		bPressedLastFrame = false;
	}
}
void toggleBoundingBoxCube(GLFWwindow* window) {
	static bool vPressedLastFrame = false;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		if (!vPressedLastFrame) {
			isCubeBoundingBoxVisible = !isCubeBoundingBoxVisible;
		}
		vPressedLastFrame = true;
	}
	else {
		vPressedLastFrame = false;
	}
}
void drawBoidBoundingBoxes() {
	if (!isBoidBoundingBoxVisible) return;
	for (auto& boid : boids) {
		float horizontalAngle = boid.getHorizontalAngle();
		float verticalAngle = boid.getVerticalAngle();
		glm::mat4 boidModelMatrix = glm::translate(glm::mat4(), boid.getPosition()) *
			glm::rotate(glm::mat4(1.0f), horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), verticalAngle, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::vec3(0.01f));
		drawBoundingBox(calculateBoundingBox(airplaneData.localBBox, boidModelMatrix), glm::vec3(1.0f, 0.0f, 0.0f));
	}
}
void drawCubeBoundingBoxes() {
	if (!isCubeBoundingBoxVisible) return;
	glm::mat4 cubeModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	drawCubeFrames(cubeModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
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
	drawRocks();
	//drwaBoids();

	// ------- Boids --------
	if (!isPaused) {
		for (auto& boid : boids) {
			boid.update(boids, collidableObjects);
		}
	}

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
		drawObjectTexturedNormal(models::paperplaneContext, boidmodelMatrix, boidTexture, texture::paperNormalMap);
		
	}
	drawBoidBoundingBoxes();	// B
	drawCubeBoundingBoxes();	// V
	drawDecorations();	// Tent, Bench, Vase

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
	initRocks();
	initDecorations();

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
	texture::paper = Core::LoadTexture("textures/paper/paper_1.jpg");
	texture::paper2 = Core::LoadTexture("textures/paper/paper_2.png");
	texture::paper3 = Core::LoadTexture("textures/paper/paper_3.png");
	texture::paper4 = Core::LoadTexture("textures/paper/paper_4.png");
	texture::paper5 = Core::LoadTexture("textures/paper/paper_5.png");
	texture::paperNormalMap = Core::LoadTexture("textures/paper/paper_normal.png");
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

	handlePause(window);			// P
	toggleBoundingBoxBoids(window); // B
	toggleBoundingBoxCube(window);  // V
	handleBoidInteraction(window, boids, collidableObjects); // LMB | RMB | Scroll
	handleNormalMapToggle(window);	// N
	updateDrone(window);			// WASD | QE | CTRL SPACE
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