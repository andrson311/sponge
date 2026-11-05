#pragma once

#include <list>
#include <initializer_list>
#include <type_traits>
#include <GL/glew.h>
#include "util.h"

class UniformBase
{
public:
    virtual ~UniformBase() = default;
    virtual bool Init(GLuint program, const char *pName) = 0;
};

template <typename T>
class Uniform : public UniformBase
{
public:
    bool Init(GLuint program, const char *pName) override
    {
        Loc = glGetUniformLocation(program, pName);
        if (Loc == INVALID_UNIFORM_LOCATION)
        {
            fprintf(stderr,
                    "Warning! Unable to get the location of uniform '%s'\n",
                    pName);
            return false;
        }

        return true;
    }

    void Set(const T &val) const
    {
        if constexpr (std::is_enum_v<T>)
        {
            glUniform1i(Loc, static_cast<GLint>(val));
        }
        else
        {
            static_assert(sizeof(T) == 0, "This type not implemented\n");
        }
    };

private:
    GLuint Loc = INVALID_UNIFORM_LOCATION;
};

template <typename T, size_t N>
class UniformArray : public UniformBase
{
public:
    bool Init(GLuint Program, const char* pName) override
    {
        for (size_t i = 0; i < N; i++) {
            char FullName[256];
            snprintf(FullName, sizeof(FullName), "%s[%zu]", pName, i);
            if (!m_Uniforms[i].Init(Program, FullName)) {
                return false;
            }
        }
        return true;
    }

    void Set(size_t Index, const T& Val) const
    {
        if (Index >= N) {   // matches original's active bounds check, not just assert
            return;
        }
        m_Uniforms[Index].Set(Val);
    }

private:
    Uniform<T> m_Uniforms[N];
};

template <>
inline void Uniform<glm::mat4>::Set(const glm::mat4 &val) const
{
    glUniformMatrix4fv(Loc, 1, GL_FALSE, glm::value_ptr(val));
}

template <>
inline void Uniform<glm::mat3>::Set(const glm::mat3 &val) const
{
    glUniformMatrix3fv(Loc, 1, GL_FALSE, glm::value_ptr(val));
}

template <>
inline void Uniform<glm::vec4>::Set(const glm::vec4 &val) const
{
    glUniform4f(Loc, val.x, val.y, val.z, val.w);
}

template <>
inline void Uniform<glm::vec3>::Set(const glm::vec3 &val) const
{
    glUniform3f(Loc, val.x, val.y, val.z);
}

template <>
inline void Uniform<bool>::Set(const bool &val) const
{
    glUniform1i(Loc, (int)val);
}

template <>
inline void Uniform<int>::Set(const int &val) const
{
    glUniform1i(Loc, val);
}

template <>
inline void Uniform<float>::Set(const float &val) const
{
    glUniform1f(Loc, val);
}

class Technique
{
public:
    Technique();
    virtual ~Technique();
    virtual bool Init();
    void Enable();
    GLuint GetProgram() const { return m_shaderProg; }

protected:
    bool AddShader(GLenum ShaderType, const char *pFilename);
    bool Finalize();
    GLint GetUniformLocation(const char *pUniformName);

    struct ShaderFile
    {
        GLenum type;
        const char *filename;
    };

    struct UniformBinding
    {
        UniformBase *ptr;
        const char *name;
    };

    bool InitShaders(std::initializer_list<ShaderFile> shaders);
    bool InitUniforms(std::initializer_list<UniformBinding> uniforms);

    GLuint m_shaderProg = 0;

private:
    void PrintUniformList();
    typedef std::list<GLuint> ShaderObjList;
    ShaderObjList m_shaderObjList;
};
