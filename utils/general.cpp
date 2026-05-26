#include "general.h"
#include <stdio.h>
#include <string>
#include <fstream>

bool ReadFile(const char* pFileName, std::string& outFile) {
    std::ifstream f(pFileName);

    bool ret = false;

    if (f.is_open()) {
        std::string line;
        while (getline(f, line)) {
            outFile.append(line);
            outFile.append("\n");
        }

        f.close();

        ret = true;
    } else {
        printf("Error: '%s' is cooked\n", pFileName);
    }

    return ret;
}