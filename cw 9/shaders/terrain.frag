#version 330 core
out vec4 FragColor;

in float Height; // Pass height from vertex shader

void main()
{
    // Height-based coloring
    vec3 color;
    if (Height < 0.5) {
        color = vec3(0.0, 0.0, 0.8); // Water (blue)
    } else if (Height < 1.5) {
        color = vec3(0.1, 0.6, 0.1); // Grass (green)
    } else {
        color = vec3(1.0, 1.0, 1.0); // Snow (white)
    }

    FragColor = vec4(color, 1.0);
}