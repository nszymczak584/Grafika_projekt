#version 330 core

layout (location = 0) in vec3 aPos; // Pozycja wierzcho³ka

uniform mat4 transformation;
uniform mat4 modelMatrix;

void main()
{
    gl_Position = transformation * modelMatrix * vec4(aPos, 1.0);
}