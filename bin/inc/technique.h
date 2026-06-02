#pragma once

#include <list>
#include <GL/glew.h>

class Technique
{
public:
    Technique();
    virtual ~Technique();
    virtual bool Init();
    void Enable();
    GLuint GetProgram() const { return m_shaderProg; }

protected:
    bool AddShader(GLenum ShaderType, const char* pFilename);
    bool Finalize();
    GLint GetUniformLocation(const char* pUniformName);
    GLuint m_shaderProg = 0;

private:
    void PrintUniformList();
    typedef std::list<GLuint> ShaderObjList;
    ShaderObjList m_shaderObjList;
};
