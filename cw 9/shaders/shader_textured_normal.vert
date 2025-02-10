#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;
layout(location = 3) in vec3 vertexTangent;

uniform mat4 transformation;
uniform mat4 modelMatrix;
uniform mat4 LightVP;

uniform vec3 sunPos;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 cameraPos;

out vec3 worldPos;
out vec3 vecNormal;
out vec2 vecTex;
out mat3 TBN; // Tangent-Bitangent-Normal matrix for normal mapping
out vec4 sunSpacePos;

void main()
{
    // World position calculation
    worldPos = vec3(modelMatrix * vec4(vertexPosition, 1.0));

    // Normal matrix for transforming normal vectors correctly in the world space
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vecNormal = normalize(normalMatrix * vertexNormal);

    // Tangent space calculations
    vec3 N = normalize(vec3(modelMatrix * vec4(vertexNormal, 0.0)));
    vec3 T = normalize(vec3(modelMatrix * vec4(vertexTangent, 0.0)));
    T = normalize(T - dot(T, N) * N); // Re-orthogonalize tangent
    vec3 B = cross(N, T); // Bitangent

    // Calculate TBN matrix
    TBN = mat3(T, B, N);

    // Texture coordinates, optionally flipped vertically
    vecTex = vec2(vertexTexCoord.x, 1.0 - vertexTexCoord.y);

    // Compute sun space position for shadow mapping
    sunSpacePos = LightVP * modelMatrix * vec4(vertexPosition, 1.0);

    // Final vertex position in clip space
    gl_Position = transformation * vec4(vertexPosition, 1.0);
}
