#version 430 core

uniform sampler2D colorTexture;
uniform sampler2D normalMap;
uniform vec3 sunDir;
uniform mat4 modelMatrix;
uniform float ambientLight; // Dynamiczny poziom œwiat³a otoczenia

in vec2 vecTex;
in vec3 worldPos;
in mat3 TBN; // Macierz TBN dla transformacji normalnych

out vec4 outColor;

void main()
{
    vec3 lightDir = normalize(sunDir);

    // Pobranie normalnej z normal mapy w zakresie [0,1]
    vec3 normalColor = texture(normalMap, vecTex).rgb;
    
    // Konwersja z [0,1] na [-1,1]
    vec3 mappedNormal = normalize(TBN * (2.0 * normalColor - 1.0));

    vec4 textureColor = texture(colorTexture, vecTex);

    if (textureColor.a < 0.2) // Jeœli tekstura ma przezroczystoœæ, odrzucamy piksel
        discard;

    float diffuse = max(0.0, dot(mappedNormal, lightDir));

    // Dynamiczne oœwietlenie na podstawie ambientLight
    vec3 lighting = min(1.0, ambientLight + diffuse) * textureColor.rgb;
    
    outColor = vec4(lighting, textureColor.a);
}
