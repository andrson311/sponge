#pragma once

#include <GL/glew.h>

class GLBuffer
{
public:
    GLBuffer() {}
    ~GLBuffer() {}

    void InitBuffer(GLsizeiptr size, const void *data, GLbitfield flags);
    void BindUBO(int bindIndex);
    void BindSSBO(int bindIndex);

    void *MapForReading();
    void Unmap();
    void Update(const void *pData, size_t size);

private:
    GLuint m_buf = 0;
    size_t m_size = 0;
};
