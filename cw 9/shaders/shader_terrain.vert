#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 LightVP; // Light projection * view matrix
uniform float textureScale;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 SunSpacePos; // Position in light space

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // World-space position
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform normal to world space
    TexCoord = aTexCoord * textureScale; // Scale texture coordinates

    SunSpacePos = LightVP * vec4(FragPos, 1.0); // Transform to light space

    gl_Position = mvp * vec4(aPos, 1.0);
}
