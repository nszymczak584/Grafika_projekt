#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord; // Texture coordinates from the vertex shader

uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 cameraPos;
uniform sampler2D terrainTexture; // Texture sampler

void main()
{
    // Sample the texture
    vec4 textureColor = texture(terrainTexture, TexCoord);

    
    vec3 finalColor = textureColor.rgb ;

    // Lighting calculations
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(sunDir);

    // Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * sunColor;

    // Specular lighting (optional)
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * sunColor;

    // Combine lighting with final color
    vec3 result = (diffuse + specular) * finalColor;

    FragColor = vec4(result, 1.0);
}