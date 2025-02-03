#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 mvp;

out float Height; // Pass height to fragment shader

void main()
{
    Height = aPos.y; // Use the Y coordinate as height
    gl_Position = mvp * vec4(aPos, 1.0);
}