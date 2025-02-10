#version 430 core

uniform sampler2D depthMap;
uniform sampler2D colorTexture;
uniform sampler2D normalMap;

uniform vec3 cameraPos;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 sunPos;
uniform float ambientLight; // Dynamic ambient light intensity

in vec2 vecTex;
in vec3 worldPos;
in mat3 TBN; // Tangent-Bitangent-Normal matrix for normal mapping
in vec4 sunSpacePos;

out vec4 outColor;

float calculateShadow(vec4 sunSpacePos, vec3 normal, vec3 sunDir) {
    // Normalize the coordinates
    vec3 sunSpacePosNormalized = sunSpacePos.xyz / sunSpacePos.w;

    // Scale to [0, 1] range
    sunSpacePosNormalized = sunSpacePosNormalized * 0.5 + 0.5;

    // Get depth from the depth map
    float closestDepth = texture(depthMap, sunSpacePosNormalized.xy).r;

    // Current depth of the fragment
    float currentDepth = sunSpacePosNormalized.z;

    // Calculate bias based on the angle between the normal and light direction
    float bias = max(0.01 * (1.0 - dot(normal, sunDir)), 0.001);

    // Check if the fragment is in shadow
    return (currentDepth > closestDepth + bias) ? 1.0 : 0.0;
}

void main()
{
    // Sample normal map and apply TBN transformation
    vec3 normalColor = texture(normalMap, vecTex).rgb;
    vec3 mappedNormal = normalize(TBN * (2.0 * normalColor - 1.0)); // Convert to [-1,1]

    // Get the color from the main texture
    vec4 textureColor = texture(colorTexture, vecTex);

    // Discard fragments with transparency
    if (textureColor.a < 0.2)
        discard;

    // Ambient lighting based on the global ambientLight parameter
    vec3 ambient = ambientLight * sunColor;

    // Direction of the light (sun)
    vec3 lightDir = normalize(sunPos - worldPos);

    // Diffuse lighting component
    float diff = max(0.0, dot(mappedNormal, lightDir));
    vec3 diffuse = diff * sunColor;

    // Specular lighting component
    vec3 viewDir = normalize(cameraPos - worldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(mappedNormal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * sunColor;

    // Calculate shadow using the sun space position
    float shadow = calculateShadow(sunSpacePos, mappedNormal, sunDir);

    // Combine lighting components
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * textureColor.rgb;

    // Output the final color
    outColor = vec4(lighting, textureColor.a);
}
