#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 cameraPos;

void main()
{
    // Height-based coloring
    vec3 color;
    float height = FragPos.y; // Use the Y coordinate as height

    if (height < 1.2) {
        color = vec3(0.0, 0.0, 0.8); // Water (blue)
    } else if (height < 2.5) {
        color = vec3(0.1, 0.6, 0.1); // Grass (green)
    } else {
        color = vec3(1.0, 1.0, 1.0); // Snow (white)
    }

    // Lighting calculations
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-sunDir); // Sun direction

    // Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * sunColor;

    // Specular lighting (optional)
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * sunColor;

    // Combine lighting with terrain color
    vec3 result = (diffuse + specular) * color;

    FragColor = vec4(result, 1.0);
}