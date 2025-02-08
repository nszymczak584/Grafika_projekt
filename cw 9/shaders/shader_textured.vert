#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;

uniform mat4 transformation;
uniform mat4 modelMatrix;

out vec3 vecNormal;
out vec3 worldPos;
out vec2 vecTex;

void main()
{
    worldPos = vec3(modelMatrix * vec4(vertexPosition, 1.0));

    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    vecNormal = normalize(normalMatrix * vertexNormal);

    vecTex = vec2(vertexTexCoord.x, 1.0 - vertexTexCoord.y); // Opcjonalne odwr�cenie UV

    gl_Position = transformation * vec4(vertexPosition, 1.0);
}
