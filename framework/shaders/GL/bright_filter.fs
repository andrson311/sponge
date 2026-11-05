#version 420

layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2D gSampler;

in vec2 TexCoords;
uniform float gLuminanceThreshold = 0.3;

float luminance(vec3 color) {
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

void main() {
    vec4 val = texture(gSampler, TexCoords);
    float lum = luminance(val.rgb);

    if(lum > gLuminanceThreshold) {
        FragColor = vec4(val.rgb - gLuminanceThreshold, val.a);
    } else {
        FragColor = vec4(0.0);
    }
}
