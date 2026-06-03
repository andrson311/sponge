#pragma once

#include <string>

#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF

bool ReadFile(const char *pFileName, std::string &outFile);
std::string GetDirFromFilename(const std::string &Filename);