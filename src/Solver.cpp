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

    int countIntegerDigits(const std::string& token)
    {
        std::size_t start = 0;
        if (!token.empty() && token[0] == '-')
        {
            start = 1;
        }
        std::size_t dotPosition = token.find('.', start);
        const std::size_t end = (dotPosition == std::string::npos)
                                ? token.size() : dotPosition;
        return static_cast<int>(end - start);
    }

    int countFractionalDigits(const std::string& token)
    {
        std::size_t dotPosition = token.find('.');
        int result = 0;
        if (dotPosition != std::string::npos)
        {
            result = static_cast<int>(token.size() - dotPosition - 1);
        }
        return result;
    }

    std::string formatPositiveDecimal(double absValue)
    {
        std::ostringstream stream;
        stream.setf(std::ios::fixed);
        stream.precision(3);
        stream << absValue;
        std::string text = stream.str();

        std::size_t lastNonZero = text.find_last_not_of('0');
        if (lastNonZero != std::string::npos)
        {
            text.erase(lastNonZero + 1);
        }
        if (!text.empty() && text.back() == '.')
        {
            text.pop_back();
        }
        return text;
    }
}
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
    if (result && position < length && token[position] == '.')
    {
        ++position;
        const std::size_t fractionStart = position;
        while (result && position < length
               && std::isdigit(static_cast<unsigned char>(token[position])))
        {
            ++position;
        }
        if (result && position == fractionStart)
        {
            result = false;
        }
    }

    if (result && position != length)
    {
        result = false;
    }

    return result;
}

std::string formatNumber(double value)
{
    const double rounded = std::round(value);
    std::string result;
    if (std::fabs(value - rounded) < kNumericTolerance)
    {
        long long integerValue = static_cast<long long>(rounded);
        if (integerValue == 0)
        {
            result = "0";
        }
        else
        {
            std::ostringstream stream;
            stream << integerValue;
            result = stream.str();
        }
    }
    else
    {
        const double absoluteValue = std::fabs(value);
        result = formatPositiveDecimal(absoluteValue);
        if (value < 0.0)
        {
            result = "-" + result;
        }
    }
    return result;
}