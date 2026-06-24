/**
 * @file Solver.cpp
 * @brief Реализация модуля решения СЛАУ методом Гаусса-Жордана с частичным
 *        выбором ведущего элемента и сопутствующих вспомогательных функций.
 */

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
    // ── Константы спецификации ────────────────────────────────────────────

    /// Максимально допустимая длина строки во входном файле (символов).
    constexpr int kMaxLineLength = 250;

    /// Минимальное допустимое количество уравнений в системе.
    constexpr int kMinEquations = 2;

    /// Максимальное допустимое количество уравнений в системе.
    constexpr int kMaxEquations = 10;

    /// Минимальное допустимое количество неизвестных.
    constexpr int kMinUnknowns = 2;

    /// Максимальное допустимое количество неизвестных.
    constexpr int kMaxUnknowns = 10;

    /// Максимальное допустимое число цифр в целой части числа.
    constexpr int kMaxIntegerDigits = 15;

    /// Максимальное допустимое число цифр в дробной части числа.
    constexpr int kMaxFractionalDigits = 3;

    /// Порог для сравнения чисел с плавающей точкой с нулём.
    constexpr double kNumericTolerance = 1e-9;

    // ── Вспомогательные функции ───────────────────────────────────────────

    /**
     * @brief Удаляет ведущие и завершающие пробельные символы из строки.
     * @param source Исходная строка.
     * @return Строка без пробельных символов по краям.
     */
    std::string trimWhitespace(const std::string& source)
    {
        const auto isNotSpace = [](unsigned char ch)
        {
            return std::isspace(ch) == 0;
        };
        auto firstNonSpace = std::ranges::find_if(source, isNotSpace);
        auto lastNonSpace  = std::find_if(source.rbegin(), source.rend(), isNotSpace).base();
        std::string result;
        if (firstNonSpace < lastNonSpace)
        {
            result.assign(firstNonSpace, lastNonSpace);
        }
        return result;
    }

    /**
     * @brief Разбивает строку на лексемы по пробельным символам.
     *        Несколько подряд идущих пробелов считаются одним разделителем.
     * @param line Строка для разбивки.
     * @return Вектор непустых лексем.
     */
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

    /**
     * @brief Возвращает количество цифр в целой части числа,
     *        записанного в строковом виде. Знак «-» не учитывается.
     * @param token Строковое представление числа.
     * @return Количество цифр целой части.
     */
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

    /**
     * @brief Возвращает количество цифр в дробной части числа.
     *        Если дробной части нет, возвращает 0.
     * @param token Строковое представление числа.
     * @return Количество цифр дробной части.
     */
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

    /**
     * @brief Преобразует неотрицательное значение double в строку
     *        с тремя знаками после запятой и без хвостовых нулей.
     *
     *        Используется в formatNumber после отделения знака,
     *        поэтому принимает только абсолютное значение.
     *
     * @param absValue Неотрицательное число.
     * @return Строковое представление без хвостовых нулей и точки.
     */
    std::string formatPositiveDecimal(double absValue)
    {
        std::ostringstream stream;
        stream.setf(std::ios::fixed);
        stream.precision(3);
        stream << absValue;
        std::string text = stream.str();

        // Удаляем хвостовые нули ("1.500" → "1.5").
        std::size_t lastNonZero = text.find_last_not_of('0');
        if (lastNonZero != std::string::npos)
        {
            text.erase(lastNonZero + 1);
        }

        // Если осталась завершающая точка — удаляем её ("2." → "2").
        if (!text.empty() && text.back() == '.')
        {
            text.pop_back();
        }

        return text;
    }

    /**
     * @brief Находит индекс строки с максимальным по модулю элементом
     *        в столбце col, начиная со строки startRow.
     *
     *        Используется для частичного выбора ведущего элемента,
     *        что повышает численную устойчивость метода.
     *
     * @param matrix   Текущее состояние расширенной матрицы.
     * @param col      Рассматриваемый столбец.
     * @param startRow Начальная строка поиска.
     * @return Индекс строки с максимальным |element| в столбце col.
     */
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

    /**
     * @brief Делит все элементы строки rowIndex на ведущий элемент
     *        в столбце pivotCol, приводя его к единице.
     *
     * @param matrix    Матрица, изменяемая на месте.
     * @param rowIndex  Индекс нормализуемой строки.
     * @param pivotCol  Столбец ведущего элемента.
     */
    void normalizeRow(Matrix& matrix, int rowIndex, int pivotCol)
    {
        const double pivotValue = matrix.data[rowIndex][pivotCol];
        for (int col = 0; col < matrix.cols; ++col)
        {
            matrix.data[rowIndex][col] /= pivotValue;
        }
    }

    /**
     * @brief Обнуляет столбец pivotCol во всех строках, кроме pivotRow,
     *        вычитая из каждой строки строку pivotRow, умноженную
     *        на соответствующий коэффициент.
     *
     *        Обнуляет строки как ниже, так и выше ведущей — тем самым
     *        реализует шаг обратного хода метода Гаусса-Жордана.
     *
     * @param matrix    Матрица, изменяемая на месте.
     * @param pivotRow  Индекс ведущей строки.
     * @param pivotCol  Индекс ведущего столбца.
     */
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

    /**
     * @brief Анализирует приведённую матрицу и определяет тип решения СЛАУ.
     *
     *        Правила классификации:
     *        - Если найдена строка вида [0 0 ... 0 | b] с b ≠ 0 — противоречие,
     *          система не имеет решений.
     *        - Если число ненулевых строк меньше числа неизвестных — система
     *          имеет бесконечно много решений.
     *        - Иначе — единственное решение.
     *
     * @param matrix Матрица после прямого хода элиминации.
     * @return Тип решения: uniqueSolution, noSolutions или
     *         infinitelyManySolutions.
     */
    SolutionType classifySolution(const Matrix& matrix)
    {
        const int unknownsCount = matrix.cols - 1;

        bool foundContradiction = false;
        int nonZeroRows = 0;

        for (int row = 0; row < matrix.rows; ++row)
        {
            // Проверяем, являются ли все коэффициенты строки нулевыми.
            bool allCoefficientsZero = true;
            for (int col = 0; col < unknownsCount; ++col)
            {
                if (std::fabs(matrix.data[row][col]) > kNumericTolerance)
                {
                    allCoefficientsZero = false;
                }
            }

            const double freeMember = matrix.data[row][unknownsCount];

            // Строка [0 ... 0 | b≠0] означает противоречие.
            if (allCoefficientsZero && std::fabs(freeMember) > kNumericTolerance)
            {
                foundContradiction = true;
            }

            // Считаем строки с хотя бы одним ненулевым коэффициентом.
            if (!allCoefficientsZero)
            {
                ++nonZeroRows;
            }
        }

        SolutionType result = SolutionType::uniqueSolution;
        if (foundContradiction)
        {
            result = SolutionType::noSolutions;
        }
        else if (nonZeroRows < unknownsCount)
        {
            result = SolutionType::infinitelyManySolutions;
        }
        return result;
    }

    /**
     * @brief Извлекает значения неизвестных из матрицы, приведённой к
     *        единичной диагональной форме.
     *
     *        Для каждого неизвестного x_i находит строку, в которой
     *        элемент в столбце i равен 1 (с допуском kNumericTolerance),
     *        и берёт значение свободного члена из этой строки.
     *
     * @param matrix Матрица после полного хода элиминации.
     * @return Вектор значений неизвестных x_0, x_1, ..., x_{n-1}.
     */
    std::vector<double> extractSolution(const Matrix& matrix)
    {
        const int unknownsCount = matrix.cols - 1;
        std::vector<double> solution(unknownsCount, 0.0);
        for (int unknownIndex = 0; unknownIndex < unknownsCount; ++unknownIndex)
        {
            for (int row = 0; row < matrix.rows; ++row)
            {
                if (std::fabs(matrix.data[row][unknownIndex] - 1.0) < kNumericTolerance)
                {
                    solution[unknownIndex] = matrix.data[row][unknownsCount];
                }
            }
        }
        return solution;
    }

    /**
     * @brief Проверяет одну лексему и при необходимости добавляет ошибку
     *        в errors. Если лексема корректна — добавляет распарсенное
     *        число в rowValues.
     *
     *        Порядок проверок: синтаксис числа → длина целой части →
     *        длина дробной части → преобразование stod.
     *
     * @param token      Строковое представление числа.
     * @param rowNumber  Номер строки во входном файле (для сообщения об ошибке).
     * @param colNumber  Номер столбца (позиция токена в строке).
     * @param rowValues  Выходной вектор, пополняемый при успешном разборе.
     * @param errors     Вектор ошибок, пополняемый при неудаче.
     */
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

    /**
     * @brief Обрабатывает одну строку файла: проверяет длину и пустоту,
     *        разбирает лексемы и собирает числовые значения в rowValues.
     *
     *        Устанавливает addRowToMatrix в true только если все токены
     *        строки успешно преобразованы в числа.
     *
     * @param rawLine        Необработанная строка из файла.
     * @param rowNumber      Номер строки (для сообщений об ошибках).
     * @param rowValues      Выходной вектор числовых значений строки.
     * @param errors         Вектор ошибок, пополняемый при необходимости.
     * @param addRowToMatrix Выходной флаг: добавлять ли строку в матрицу.
     */
    void processLine(const std::string& rawLine,
                     int rowNumber,
                     std::vector<double>& rowValues,
                     std::vector<Error>& errors,
                     bool& addRowToMatrix)
    {
        addRowToMatrix = false;
        const int rawLength = static_cast<int>(rawLine.size());

        // Строка длиннее допустимого предела.
        if (rawLength > kMaxLineLength)
        {
            errors.emplace_back(ErrorType::lineTooLong, "",
                                rowNumber, -1, rawLength);
        }
        else
        {
            const std::string trimmed = trimWhitespace(rawLine);

            // Строка пустая или состоит только из пробелов.
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
                // Добавляем строку в матрицу только если все токены корректны.
                addRowToMatrix = (static_cast<int>(rowValues.size())
                                  == static_cast<int>(tokens.size()));
            }
        }
    }

    /**
     * @brief Выполняет проверки размерности собранной матрицы:
     *        единообразие числа столбцов, допустимые диапазоны строк
     *        и неизвестных. Добавляет ошибки в errors при нарушениях.
     *
     * @param matrix Матрица после разбора всех строк файла.
     * @param errors Вектор ошибок, пополняемый при необходимости.
     */
    void validateDimensions(const Matrix& matrix, std::vector<Error>& errors)
    {
        if (matrix.rows == 0)
        {
            errors.emplace_back(ErrorType::emptyFile);
        }
        else
        {
            // Проверяем, что все строки содержат одинаковое число столбцов.
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
                // Проверяем допустимое число уравнений.
                if (matrix.rows < kMinEquations)
                {
                    errors.emplace_back(ErrorType::tooFewRows);
                }
                if (matrix.rows > kMaxEquations)
                {
                    errors.emplace_back(ErrorType::tooManyRows);
                }

                // Столбцов матрицы на один больше числа неизвестных
                // (последний столбец — свободные члены).
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

} // namespace

// ── Публичные функции ─────────────────────────────────────────────────────────

/**
 * @brief Проверяет, является ли строка token допустимым числом
 *        согласно спецификации входного формата.
 *
 *        Допустимый формат: [-]D{1..15}[.D{1..3}], где D — десятичная цифра.
 *        Знак «+», экспоненциальная запись и пробелы не допускаются.
 *
 * @param token Строковое представление предполагаемого числа.
 * @return true  если token является допустимым числом,
 *         false в противном случае.
 */
bool isValidNumber(const std::string& token)
{
    bool result = true;
    std::size_t position = 0;
    const std::size_t length = token.size();

    // Пустая строка — не число.
    if (length == 0)
    {
        result = false;
    }

    // Необязательный знак минус.
    if (result && position < length && token[position] == '-')
    {
        ++position;
    }

    // Целая часть: хотя бы одна цифра обязательна.
    const std::size_t integerStart = position;
    while (result && position < length
           && std::isdigit(static_cast<unsigned char>(token[position])))
    {
        ++position;
    }
    if (result && position == integerStart)
    {
        result = false; // нет ни одной цифры (или строка была только "-")
    }

    // Необязательная дробная часть: точка + хотя бы одна цифра.
    if (result && position < length && token[position] == '.')
    {
        ++position;
        const std::size_t fractionStart = position;
        while (position < length
               && std::isdigit(static_cast<unsigned char>(token[position])))
        {
            ++position;
        }
        if (position == fractionStart)
        {
            result = false; // точка без цифр после неё ("5." недопустимо)
        }
    }

    // Если остались необработанные символы — строка не является числом
    // (буквы, «+», вторая точка и т.д.).
    if (result && position != length)
    {
        result = false;
    }

    return result;
}

/**
 * @brief Форматирует число double в строку согласно правилам вывода:
 *        целые числа выводятся без дробной части,
 *        дробные — с точностью до трёх знаков без хвостовых нулей.
 *
 *        Особые случаи: -0.0 и значения, отличающиеся от нуля
 *        менее чем на kNumericTolerance, выводятся как "0".
 *
 * @param value Число для форматирования.
 * @return Строковое представление числа.
 */
std::string formatNumber(double value)
{
    const double rounded = std::round(value);
    std::string result;

    if (std::fabs(value - rounded) < kNumericTolerance)
    {
        // Значение близко к целому — выводим без дробной части.
        // Для -0.0, округлившегося к 0, явно возвращаем "0".
        if (static_cast<long long>(rounded) == 0)
        {
            result = "0";
        }
        else
        {
            std::ostringstream stream;
            stream << static_cast<long long>(rounded);
            result = stream.str();
        }
    }
    else
    {
        // Значение дробное — форматируем абсолютную величину,
        // затем при необходимости добавляем знак минус.
        const double absoluteValue = std::fabs(value);
        result = formatPositiveDecimal(absoluteValue);
        if (value < 0.0)
        {
            result = "-" + result;
        }
    }
    return result;
}

/**
 * @brief Решает систему линейных алгебраических уравнений методом
 *        Гаусса-Жордана с частичным выбором ведущего элемента.
 *
 *        Алгоритм работает непосредственно с переданной матрицей,
 *        изменяя её содержимое в процессе вычислений.
 *
 *        После завершения:
 *        - при uniqueSolution вектор solution содержит значения x_0..x_{n-1};
 *        - при noSolutions и infinitelyManySolutions solution пуст.
 *
 * @param matrix   Расширенная матрица коэффициентов [A|b], изменяется на месте.
 * @param solution Выходной вектор решения (заполняется при uniqueSolution).
 * @return Тип решения: uniqueSolution, noSolutions или
 *         infinitelyManySolutions.
 */
SolutionType gaussianElimination(Matrix& matrix,
                                 std::vector<double>& solution)
{
    solution.clear();

    const int unknownsCount = matrix.cols - 1;

    // Прямой ход: приводим матрицу к единичной диагональной форме.
    // leadRow — индекс текущей ведущей строки.
    int leadRow = 0;
    for (int col = 0; col < unknownsCount && leadRow < matrix.rows; ++col)
    {
        // Выбираем строку с максимальным по модулю элементом в текущем столбце.
        const int pivotRow = findPivotRow(matrix, col, leadRow);
        const double pivotValue = std::fabs(matrix.data[pivotRow][col]);

        // Если ведущий элемент близок к нулю — столбец вырожден, пропускаем.
        if (pivotValue >= kNumericTolerance)
        {
            std::swap(matrix.data[leadRow], matrix.data[pivotRow]);
            normalizeRow(matrix, leadRow, col);    // ведущий элемент → 1
            eliminateColumn(matrix, leadRow, col); // обнуляем остальные строки
            ++leadRow;
        }
    }

    // Определяем тип решения и извлекаем его при необходимости.
    const SolutionType solutionType = classifySolution(matrix);
    if (solutionType == SolutionType::uniqueSolution)
    {
        solution = extractSolution(matrix);
    }
    return solutionType;
}

/**
 * @brief Читает расширенную матрицу коэффициентов из текстового файла.
 *
 *        Обрабатывает все строки файла, собирая ошибки без немедленного
 *        прерывания — пользователь получает полный список проблем за раз.
 *        Размерность матрицы проверяется только если при разборе строк
 *        ошибок не возникло (чтобы избежать каскадных дублирующих сообщений).
 *
 * @param inputPath Путь к входному файлу.
 * @param matrix    Выходная матрица (заполняется при успехе).
 * @param errors    Выходной вектор ошибок (пуст при успехе).
 * @return true  если файл успешно прочитан и матрица корректна,
 *         false при любой ошибке.
 */
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

        // Построчно читаем файл и разбираем каждую строку.
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

        // Заполняем поля размерности матрицы.
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

/**
 * @brief Записывает результат решения СЛАУ в текстовый файл.
 *
 *        Форматы вывода:
 *        - noSolutions           → "no solutions"
 *        - infinitelyManySolutions → "infinitely many solutions"
 *        - uniqueSolution        → значения через пробел, например "1 -2 3.5"
 *
 * @param outputPath Путь к выходному файлу.
 * @param type       Тип найденного решения.
 * @param solution   Вектор значений неизвестных (используется при uniqueSolution).
 * @return true  если файл успешно создан и записан,
 *         false если файл не удалось открыть на запись.
 */
bool writeResult(const std::string& outputPath,
                 SolutionType type,
                 const std::vector<double>& solution)
{
    std::ofstream stream(outputPath);
    bool result = true;

    if (!stream.is_open())
    {
        result = false;
    }
    else
    {
        switch (type)
        {
            case SolutionType::noSolutions:
                stream << "no solutions";
                break;

            case SolutionType::infinitelyManySolutions:
                stream << "infinitely many solutions";
                break;

            case SolutionType::uniqueSolution:
            {
                // Значения разделяются пробелом; перед первым пробел не ставится.
                for (std::size_t index = 0; index < solution.size(); ++index)
                {
                    if (index != 0)
                    {
                        stream << ' ';
                    }
                    stream << formatNumber(solution[index]);
                }
                break;
            }
        }
    }
    return result;
}