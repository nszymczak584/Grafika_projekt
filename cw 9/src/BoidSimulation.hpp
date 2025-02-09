#include "glew.h"
#include "Render_Utils.h"
#include "Shader_Loader.h"
#include "Texture.h"
#include <GLFW/glfw3.h>

#include "boid.h"
#include "BoidInteraction.h"
#include "Drone.h"
#include "NormalMapToggle.h"
#include "Skybox.h"
#include "Temple.h"
#include "Terrain.h"
#include "Tree.h"

int WIDTH, HEIGHT;


GLuint program;
GLuint programTexturedNormal;

std::vector<Boid> boids;
namespace models {
	Core::RenderContext paperplaneContext;
}

glm::vec3 sunDir = glm::normalize(glm::vec3(0.228586f, 0.584819f, -0.778293f));
glm::vec3 sunColor = glm::vec3(0.8f, 0.8f, 0.6f) * 4.0f;
float exposition = 2.f;
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
	for (auto& boid : boids) boid.update(boids);

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
	programTexturedNormal = Core::Shader_Loader().CreateProgram("shaders/shader_textured_normal.vert", "shaders/shader_textured_normal.frag");

	// **Generowanie boidów**
	for (int groupId = 0; groupId < 6; ++groupId) {
		for (int i = 0; i < 40; ++i) {
			glm::vec3 position(
				(rand() % 200 + 0.1f) / 100.f,
				(rand() % 200 + 0.1f) / 100.f,
				(rand() % 200 + 0.1f) / 100.f
			);
			boids.push_back(Boid(position, groupId, i));
		}
	}
	loadModelToContext("./models/paperAirplane.obj", models::paperplaneContext);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
	
	handleBoidInteraction(window, boids);
	handleNormalMapToggle(window);
	updateDrone(window);
	updateCamera();
}

void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window)){
		processInput(window);
		renderScene(window);
		glfwPollEvents();
	}
}

void shutdown(GLFWwindow* window) {
	Core::Shader_Loader().DeleteProgram(program);
}