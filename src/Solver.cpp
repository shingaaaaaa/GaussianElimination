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

    std::string trimWhitespace(const std::string& source)
    {
        const auto isNotSpace = [](unsigned char ch)
        {
            return std::isspace(ch) == 0;
        };
        auto firstNonSpace = std::find_if(source.begin(), source.end(), isNotSpace);
        auto lastNonSpace = std::find_if(source.rbegin(), source.rend(), isNotSpace).base();
        std::string result;
        if (firstNonSpace < lastNonSpace)
        {
            result.assign(firstNonSpace, lastNonSpace);
        }
        return result;
    }

    std::vector<std::string> splitIntoTokens(const std::string& line)
    {
        std::vector<std::string> tokens;
        std::istringstream stream(line);
        std::string token;
        while (stream >> token)
        {
            tokens.push_back(token);
        }
        return tokens;
    }

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

    int findPivotRow(const Matrix& matrix, int col, int startRow)
    {
        int pivotRow = startRow;
        double pivotMagnitude = std::fabs(matrix.data[startRow][col]);
        for (int row = startRow + 1; row < matrix.rows; ++row)
        {
            const double currentMagnitude = std::fabs(matrix.data[row][col]);
            if (currentMagnitude > pivotMagnitude)
            {
                pivotMagnitude = currentMagnitude;
                pivotRow = row;
            }
        }
        return pivotRow;
    }

    void normalizeRow(Matrix& matrix, int rowIndex, int pivotCol)
    {
        const double pivotValue = matrix.data[rowIndex][pivotCol];
        for (int col = 0; col < matrix.cols; ++col)
        {
            matrix.data[rowIndex][col] /= pivotValue;
        }
    }
    void eliminateColumn(Matrix& matrix, int pivotRow, int pivotCol)
    {
        for (int row = 0; row < matrix.rows; ++row)
        {
            if (row != pivotRow)
            {
                const double factor = matrix.data[row][pivotCol];
                for (int col = 0; col < matrix.cols; ++col)
                {
                    matrix.data[row][col] -= factor * matrix.data[pivotRow][col];
                }
            }
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

SolutionType gaussianElimination(Matrix& matrix,
                                 std::vector<double>& solution)
{
    solution.clear();
    const int unknownsCount = matrix.cols - 1;

    int leadRow = 0;
    for (int col = 0; col < unknownsCount && leadRow < matrix.rows; ++col)
    {
        const int pivotRow = findPivotRow(matrix, col, leadRow);
        const double pivotValue = std::fabs(matrix.data[pivotRow][col]);
        if (pivotValue >= kNumericTolerance)
        {
            std::swap(matrix.data[leadRow], matrix.data[pivotRow]);
            normalizeRow(matrix, leadRow, col);
            eliminateColumn(matrix, leadRow, col);
            ++leadRow;
        }
    }

    return SolutionType::uniqueSolution;
}

namespace
{
    void processToken(const std::string& token,
                      int rowNumber,
                      int colNumber,
                      std::vector<double>& rowValues,
                      std::vector<Error>& errors)
    {
        if (!isValidNumber(token))
        {
            errors.emplace_back(ErrorType::nonNumericValue,
                                token, rowNumber, colNumber);
        }
        else if (countIntegerDigits(token) > kMaxIntegerDigits)
        {
            errors.emplace_back(ErrorType::integerPartTooLong,
                                token, rowNumber, colNumber);
        }
        else if (countFractionalDigits(token) > kMaxFractionalDigits)
        {
            errors.emplace_back(ErrorType::tooManyDecimalPlaces,
                                token, rowNumber, colNumber);
        }
        else
        {
            try
            {
                rowValues.push_back(std::stod(token));
            }
            catch (const std::exception&)
            {
                // Дополнительная защита: если stod выбросил исключение
                // несмотря на пройденные проверки — фиксируем как
                // нечисловое значение.
                errors.emplace_back(ErrorType::nonNumericValue,
                                    token, rowNumber, colNumber);
            }
        }
    }
    void processLine(const std::string& rawLine,
                     int rowNumber,
                     std::vector<double>& rowValues,
                     std::vector<Error>& errors,
                     bool& addRowToMatrix)
    {
        addRowToMatrix = false;
        const int rawLength = static_cast<int>(rawLine.size());
        if (rawLength > kMaxLineLength)
        {
            errors.emplace_back(ErrorType::lineTooLong, "",
                                rowNumber, -1, rawLength);
        }
        else
        {
            const std::string trimmed = trimWhitespace(rawLine);
            if (trimmed.empty())
            {
                errors.emplace_back(ErrorType::emptyLine, "", rowNumber);
            }
            else
            {
                const std::vector<std::string> tokens = splitIntoTokens(trimmed);
                for (std::size_t tokenIndex = 0;
                     tokenIndex < tokens.size();
                     ++tokenIndex)
                {
                    processToken(tokens[tokenIndex],
                                 rowNumber,
                                 static_cast<int>(tokenIndex) + 1,
                                 rowValues,
                                 errors);
                }
                addRowToMatrix = (static_cast<int>(rowValues.size()) == static_cast<int>(tokens.size()));
            }
        }
    }

    void validateDimensions(const Matrix& matrix, std::vector<Error>& errors)
    {
        if (matrix.rows == 0)
        {
            errors.emplace_back(ErrorType::emptyFile);
        }
        else
        {
            // Проверяем единообразие количества столбцов.
            const std::size_t firstRowSize = matrix.data[0].size();
            bool allRowsSameSize = true;
            for (const auto& row : matrix.data)
            {
                if (row.size() != firstRowSize)
                {
                    allRowsSameSize = false;
                }
            }
            if (!allRowsSameSize)
            {
                errors.emplace_back(ErrorType::unequalColumns);
            }
            else
            {
                if (matrix.rows < kMinEquations)
                {
                    errors.emplace_back(ErrorType::tooFewRows);
                }
                if (matrix.rows > kMaxEquations)
                {
                    errors.emplace_back(ErrorType::tooManyRows);
                }
                const int unknownsCount = matrix.cols - 1;
                if (unknownsCount < kMinUnknowns)
                {
                    errors.emplace_back(ErrorType::tooFewCols);
                }
                if (unknownsCount > kMaxUnknowns)
                {
                    errors.emplace_back(ErrorType::tooManyCols);
                }
            }
        }
    }
}

bool readMatrix(const std::string& inputPath,
                Matrix& matrix,
                std::vector<Error>& errors)
{
    matrix = Matrix();
    errors.clear();

    std::ifstream stream(inputPath);
    bool result = true;
    if (!stream.is_open())
    {
        errors.emplace_back(ErrorType::inputFileNotExist);
        result = false;
    }
    else
    {
        std::string rawLine;
        int rowNumber = 0;
        while (std::getline(stream, rawLine))
        {
            ++rowNumber;
            std::vector<double> rowValues;
            bool addRow = false;
            processLine(rawLine, rowNumber, rowValues, errors, addRow);
            if (addRow)
            {
                matrix.data.push_back(rowValues);
            }
        }
        matrix.rows = static_cast<int>(matrix.data.size());
        matrix.cols = (matrix.rows > 0)
                      ? static_cast<int>(matrix.data[0].size())
                      : 0;

        // Проверка размерности имеет смысл только если строки разобраны
        // без ошибок — иначе сообщения каскадно дублируют друг друга.
        if (errors.empty())
        {
            validateDimensions(matrix, errors);
        }
        result = errors.empty();
    }
    return result;
}
