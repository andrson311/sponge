#include <stdio.h>
#include <string>
#include <fstream>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include "util.h"

long long GetCurrentTimeMillis()
{
    timeval t;
    gettimeofday(&t, NULL);

    long long ret = t.tv_sec * 1000 + t.tv_usec / 1000;
    return ret;
}

bool ReadFile(const char *pFileName, std::string &outFile)
{
    std::ifstream f(pFileName);

    bool ret = false;

    if (f.is_open())
    {
        std::string line;
        while (getline(f, line))
        {
            outFile.append(line);
            outFile.append("\n");
        }

        f.close();

        ret = true;
    }
    else
    {
        printf("Error: '%s' is cooked\n", pFileName);
    }

    return ret;
}

char *ReadBinaryFile(const char *pFilename, int &size)
{
    FILE *f = fopen(pFilename, "rb");

    if (!f)
    {
        printf("Error opening the binary file");
        exit(0);
    }

    struct stat stat_buf;
    int error = stat(pFilename, &stat_buf);

    if (error)
    {
        printf("Error getting file stats");
        return NULL;
    }

    size = stat_buf.st_size;

    char *p = (char *)malloc(size);
    assert(p);

    size_t bytes_read = fread(p, 1, size, f);
    if (bytes_read != size)
    {
        printf("Error reading file");
        exit(0);
    }

    fclose(f);

    return p;
}

std::string GetDirFromFilename(const std::string &Filename)
{
    std::string::size_type SlashIndex = Filename.find_last_of("/\\");

    if (SlashIndex == std::string::npos)
    {
        return ".";
    }
    else if (SlashIndex == 0)
    {
        return "/";
    }
    else
    {
        return Filename.substr(0, SlashIndex);
    }
}

std::string GetFullPath(const std::string &Dir, const aiString &Path)
{
    std::string p(Path.data);
    std::replace(p.begin(), p.end(), '\\', '/');

    if (p.size() >= 2 && std::isalpha((unsigned char)p[0]) && p[1] == ':')
    {
        p = p.substr(2);
    }

    if (p.size() >= 2 && p.substr(0, 2) == "./")
    {
        p = p.substr(2);
    }

    if (!p.empty() && p[0] == '/')
    {
        p = p.substr(1);
    }

    return Dir + "/" + p;
}

glm::mat4 AssimpToGLM(const aiMatrix4x4 &m)
{
    return glm::transpose(glm::make_mat4(&m.a1));
}
