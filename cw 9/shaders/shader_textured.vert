#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;

uniform mat4 transformation;
uniform mat4 modelMatrix;

uniform vec3 sunColor;
uniform vec3 sunPos;

out vec3 vecNormal;
out vec3 worldPos;
out vec2 vecTex;

uniform vec3 cameraPos;
uniform vec3 sunDir;
uniform mat4 LightVP;

out vec4 sunSpacePos;

void main()
{
    worldPos = (modelMatrix* vec4(vertexPosition,1)).xyz;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vecNormal = normalize(normalMatrix * vertexNormal);



    vecTex = vec2(vertexTexCoord.x, 1.0 - vertexTexCoord.y); // Opcjonalne odwrócenie UV

    sunSpacePos = LightVP * modelMatrix * vec4(vertexPosition, 1.0);

    gl_Position = transformation * vec4(vertexPosition, 1.0);


}
