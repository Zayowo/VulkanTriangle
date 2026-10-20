#pragma once

#include <string>
#include <vector>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

namespace Utils
{
    // Read a file and return a vector of its characters
    std::vector<char> ReadFile(const std::string& filename);
}