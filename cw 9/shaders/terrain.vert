#version 330 core
layout(location = 0) in vec3 aPos; // Vertex position
layout(location = 1) in vec3 aNormal; // Vertex normal

uniform mat4 mvp;
uniform mat4 model;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // World position
    Normal = mat3(transpose(inverse(model))) * aNormal; // Normal in world space
    gl_Position = mvp * vec4(aPos, 1.0);
}