#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 SunSpacePos;

uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 cameraPos;
uniform sampler2D terrainTexture;
uniform sampler2D depthMap;

float calculateShadow(vec4 sunSpacePos, vec3 normal, vec3 sunDir) {
    vec3 sunSpacePosNormalized = sunSpacePos.xyz / sunSpacePos.w;
    sunSpacePosNormalized = sunSpacePosNormalized * 0.5 + 0.5;

    float closestDepth = texture(depthMap, sunSpacePosNormalized.xy).r;
    float currentDepth = sunSpacePosNormalized.z;

    float bias = max(0.005 * (1.0 - dot(normal, sunDir)), 0.002);

    // PCF (Percentage Closer Filtering) for soft shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float pcfDepth = texture(depthMap, sunSpacePosNormalized.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth > pcfDepth + bias) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0; // Average over a 3x3 kernel

    return shadow;
}

void main()
{
    vec4 textureColor = texture(terrainTexture, TexCoord);
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(sunDir);

    // Ambient lighting
    vec3 ambient = 0.15 * sunColor;

    // Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * sunColor;

    // Specular lighting (optional)
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * sunColor;

    // Compute shadow
    float shadow = calculateShadow(SunSpacePos, norm, sunDir);

    // Final color with shadowing
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * textureColor.rgb;
    FragColor = vec4(lighting, 1.0);
}
