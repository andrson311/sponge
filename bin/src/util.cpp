#include "../inc/util.h"
#include <stdio.h>
#include <string>
#include <fstream>

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

std::string GetDirFromFilename(const std::string &Filename)
{
    std::string::size_type SlashIndex = Filename.find_last_of("/\\");

    if (SlashIndex == std::string::npos) {
        return ".";
    }
    else if (SlashIndex == 0) {
        return "/";
    }
    else {
        return Filename.substr(0, SlashIndex);
    }
}