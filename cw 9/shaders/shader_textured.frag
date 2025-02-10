#version 430 core


uniform sampler2D depthMap;
uniform sampler2D colorTexture;


uniform vec3 cameraPos;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 sunPos;

in vec3 vecNormal;
in vec3 worldPos;
in vec2 vecTex;
in vec4 sunSpacePos;

out vec4 outColor;

float calculateShadow(vec4 sunSpacePos, vec3 normal, vec3 sunDir) {
    // Ujednorodnij wspó³rzêdne
    vec3 sunSpacePosNormalized = sunSpacePos.xyz / sunSpacePos.w;

    // Przeskaluj do zakresu [0, 1]
    sunSpacePosNormalized = sunSpacePosNormalized * 0.5 + 0.5;

    // Pobierz g³êbokoœæ z depthMap
    float closestDepth = texture(depthMap, sunSpacePosNormalized.xy).r;

    // Aktualna g³êbokoœæ fragmentu
    float currentDepth = sunSpacePosNormalized.z;

    // Oblicz bias na podstawie k¹ta miêdzy normaln¹ a kierunkiem œwiat³a
    float bias = max(0.01 * (1.0 - dot(normal, sunDir)), 0.001);

    // SprawdŸ, czy fragment jest w cieniu
    return (currentDepth > closestDepth + bias) ? 1.0 : 0.0;
}

void main()
{
    vec4 color = texture(colorTexture, vecTex);
    vec3 normal = normalize(vecNormal);


    vec3 ambient = 0.15 * sunColor;

    if (color.a < 0.2) // Jeœli tekstura ma przezroczystoœæ, odrzucamy piksel
        discard;

    vec3 lightDir = normalize(sunPos - worldPos);
    float diff = max(0.0, dot(normal, lightDir));
    vec3 diffuse = diff * sunColor;

    vec3 viewDir = normalize(cameraPos - worldPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * sunColor;    

    float shadow = calculateShadow(sunSpacePos, normal, sunDir);

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color.rgb;    
    outColor = vec4(lighting, 1.0);
}
