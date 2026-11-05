#version 420

layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2D gSampler;

in vec2 TexCoords;
uniform float gWeights[10];

void main() {
    float dy = 1.0 / (textureSize(gSampler, 0)).y;
    vec4 sum = texture(gSampler, TexCoords) * gWeights[0];

    for(int i = 1; i < 10; i++) {
        vec2 Offset = vec2(0.0, i * dy);
        float w = gWeights[i];

        sum += texture(gSampler, TexCoords + Offset) * w;
        sum += texture(gSampler, TexCoords - Offset) * w;
    }

    FragColor = sum;
}