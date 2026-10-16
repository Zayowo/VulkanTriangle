#pragma once

#include <string>
#include <vector>

namespace Utils
{
    // Read a file and return a vector of its characters
    std::vector<char> ReadFile(const std::string& filename);
}
