#pragma once

#include <GL/glew.h>

class SaveViewport
{
public:
    SaveViewport() {}

    void Save()
    {
        glGetIntegerv(GL_VIEWPORT, m_viewport);
    }

    void Restore()
    {
        glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
    }

private:
    GLint m_viewport[4] = {0};
};