#version 430 core

const float AMBIENT = 0.1;

uniform sampler2D colorTexture;
uniform vec3 sunDir;
uniform mat4 modelMatrix;

in vec3 vecNormal;
in vec3 worldPos;
in vec2 vecTex;

out vec4 outColor;

void main()
{
    vec3 lightDir = normalize(transpose(inverse(mat3(modelMatrix))) * sunDir);
    
    vec3 normal = normalize(vecNormal);
    vec4 textureColor = texture(colorTexture, vecTex);

    if (textureColor.a < 0.2) // Jeœli tekstura ma przezroczystoœæ, odrzucamy piksel
        discard;

    float diffuse = max(0.0, dot(normal, lightDir));
    vec3 lighting = min(1.0, AMBIENT + diffuse) * textureColor.rgb;
    
    outColor = vec4(lighting, textureColor.a);
}
