#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;
layout(location = 3) in vec3 vertexTangent;

out vec2 vecTex;
out vec3 worldPos;
out mat3 TBN; // Macierz TBN dla normal mappingu

uniform mat4 transformation;
uniform mat4 modelMatrix;

void main()
{
    worldPos = vec3(modelMatrix * vec4(vertexPosition, 1.0));

    // Transformujemy wektory do przestrzeni œwiata
    vec3 N = normalize(vec3(modelMatrix * vec4(vertexNormal, 0.0)));
    vec3 T = normalize(vec3(modelMatrix * vec4(vertexTangent, 0.0)));

    // Re-ortogonalizacja tangenta wzglêdem normalnej (Gram-Schmidt)
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T); // Obliczenie bitangenta

    // Utworzenie macierzy TBN
    TBN = mat3(T, B, N);

    vecTex = vec2(vertexTexCoord.x, 1.0 - vertexTexCoord.y); // Opcjonalne odwrócenie UV

    gl_Position = transformation * vec4(vertexPosition, 1.0);
}
