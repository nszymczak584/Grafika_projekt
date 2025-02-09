#include "Camera.h"
#include "glm.hpp"
#include "ext.hpp"

float aspectRatio = 1.f;
extern glm::vec3 cameraPos;
extern glm::vec3 cameraDir;

glm::mat4 Core::createPerspectiveMatrix(float zNear, float zFar, float frustumScale){
	glm::mat4 perspective;
    perspective[0][0] = 1.f;
	perspective[1][1] = frustumScale;
	perspective[2][2] = (zFar + zNear) / (zNear - zFar);
	perspective[3][2] = (2 * zFar * zNear) / (zNear - zFar);
	perspective[2][3] = -1;
	perspective[3][3] = 0;

	return perspective;
}

glm::mat4 Core::createViewMatrix( glm::vec3 position, glm::vec3 forward, glm::vec3 up ){
	glm::vec3 side = glm::cross(forward, up);

	glm::mat4 cameraRotation;
	cameraRotation[0][0] = side.x; cameraRotation[1][0] = side.y; cameraRotation[2][0] = side.z;
	cameraRotation[0][1] = up.x; cameraRotation[1][1] = up.y; cameraRotation[2][1] = up.z;
	cameraRotation[0][2] = -forward.x; cameraRotation[1][2] = -forward.y; cameraRotation[2][2] = -forward.z;

	glm::mat4 cameraTranslation;
	cameraTranslation[3] = glm::vec4(-position, 1.0f);

	return cameraRotation * cameraTranslation;
}

glm::mat4 createPerspectiveMatrix() {
    return Core::createPerspectiveMatrix(0.05f, 200.0f, aspectRatio);
}

glm::mat4 createCameraMatrix() {
    glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir, glm::vec3(0.f, 1.f, 0.f)));
    glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide, cameraDir));
    glm::mat4 cameraRotationMatrix = glm::mat4({
        cameraSide.x, cameraSide.y, cameraSide.z, 0,
        cameraUp.x, cameraUp.y, cameraUp.z, 0,
        -cameraDir.x, -cameraDir.y, -cameraDir.z, 0,
        0., 0., 0., 1.
        });

    cameraRotationMatrix = glm::transpose(cameraRotationMatrix);
    glm::mat4 cameraMatrix = cameraRotationMatrix * glm::translate(-cameraPos);

    return cameraMatrix;
}