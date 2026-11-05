#include "gl_hdr_technique.h"

bool HDRTechnique::Init()
{
    return InitShaders({{GL_COMPUTE_SHADER, "framework/shaders/GL/hdr.cs"}});
}
