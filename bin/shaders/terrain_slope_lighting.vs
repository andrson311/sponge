#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 InTex;
layout (location = 2) in float InLightFactor;

uniform mat4 gVP;

out float LightFactor;
out vec2 Tex;
out vec3 WorldPos;

void main()
{
    gl_Position = gVP * vec4(Position, 1.0);

    LightFactor = InLightFactor;

    Tex = InTex;
    
    WorldPos = Position;
}
