#version 460 core

layout(location = 0) in vec3 Position;
uniform mat4 gWVP;
uniform mat4 gWorld;

out vec3 WorldPosition;

void main()
{
    vec4 worldPosition = gWorld * vec4(Position, 1.0);
    WorldPosition = worldPosition.xyz;
    gl_Position = gWVP * vec4(Position, 1.0);
}
