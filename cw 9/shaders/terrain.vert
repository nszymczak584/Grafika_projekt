#version 330 core
layout(location = 0) in vec3 aPos; // Vertex position
layout(location = 1) in vec3 aNormal; // Vertex normal
layout(location = 2) in vec2 aTexCoord; // Texture coordinates

uniform mat4 mvp;
uniform mat4 model;
uniform float textureScale; // Scaling factor for texture coordinates

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord; // Pass texture coordinates to the fragment shader

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // World position
    Normal = mat3(transpose(inverse(model))) * aNormal; // Normal in world space
    TexCoord = aTexCoord * textureScale; // Scale texture coordinates
    gl_Position = mvp * vec4(aPos, 1.0);
}