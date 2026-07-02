#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 InTex;

uniform float gTime;
uniform vec3 gCenter = vec3(1000.0, 0.0, 1000.0);

out VS_OUT {
    vec2 Tex;
} vs_out;


void main()
{
    gl_Position = vec4(Position, 1.0);

    float Distance = length(Position - gCenter);
    gl_Position.y = sin(Distance / 12.0 + gTime) * 8.0;

    vs_out.Tex = InTex;
}
