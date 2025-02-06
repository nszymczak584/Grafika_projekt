#version 330 core
layout(location = 0) in vec3 vertexPosition;

uniform mat4 viewProjection;

out vec3 texCoord;

void main()
{
    texCoord = vertexPosition;
    gl_Position = viewProjection * vec4(vertexPosition, 1.0);
}