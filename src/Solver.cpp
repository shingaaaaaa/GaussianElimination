#include "../include/Solver.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    constexpr int kMaxLineLength = 250;
    constexpr int kMinEquations = 2;
    constexpr int kMaxEquations = 10;
    constexpr int kMinUnknowns = 2;
    constexpr int kMaxUnknowns = 10;
    constexpr int kMaxIntegerDigits = 15;
    constexpr int kMaxFractionalDigits = 3;
    constexpr double kNumericTolerance = 1e-9;
}

bool isValidNumber(const std::string& token)
{
    bool result = true;
    std::size_t position = 0;
    const std::size_t length = token.size();

    if (length == 0)
    {
        result = false;
    }

    if (result && position < length && token[position] == '-')
    {
        ++position;
    }

    const std::size_t integerStart = position;
    while (result && position < length
           && std::isdigit(static_cast<unsigned char>(token[position])))
    {
        ++position;
    }
    if (result && position == integerStart)
    {
        result = false;
    }

    if (result && position != length)
    {
        result = false;
    }

    return result;
}