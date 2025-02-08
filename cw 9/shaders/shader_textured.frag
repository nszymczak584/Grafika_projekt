#version 430 core

const float AMBIENT = 0.1;

uniform vec3 color;
uniform vec3 lightPos;
uniform sampler2D colorTexture;

in vec3 vecNormal;
in vec3 worldPos;
in vec2 vecTex;

out vec4 outColor;

void main()
{
    vec3 lightDir = normalize(lightPos - worldPos);
    vec3 normal = normalize(vecNormal);
    vec4 textureColor = texture(colorTexture, vecTex); // Upewnij siê, ¿e masz pe³ny `vec4`

    if (textureColor.a < 0.1)  // Jeœli tekstura ma przezroczystoœæ, odrzucamy piksel
        discard;

    float diffuse = max(0.0, dot(normal, lightDir));
    outColor = vec4(textureColor.rgb * min(1.0, AMBIENT + diffuse), textureColor.a);
}
