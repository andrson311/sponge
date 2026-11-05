#version 460 core

in vec3 WorldPosition;
uniform vec3 gLightWorldPos;

void main()
{
    gl_FragDepth = length(WorldPosition - gLightWorldPos) / 100.0;
}
