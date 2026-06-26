#pragma once

#include "util.h"

class STBImage
{
public:
    STBImage() {}
    ~STBImage() {
        if (m_imageData)
        {
            Unload();
        }
    }

    void Load(const char* pFilename);
    void Unload();
    glm::vec3 GetColor(int x, int y) const;

    int m_width = 0;
    int m_height = 0;
    int m_bpp = 0;
    u_char* m_imageData = NULL;
};
