/**
 * @file Solver.cpp
 * @brief Реализация модуля решения СЛАУ методом Гаусса-Жордана с частичным
 *        выбором ведущего элемента и сопутствующих вспомогательных функций.
 */

#include "../include/Solver.h"

#include <algorithm>
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
        // лямбда, которая возвращает true если символ НЕ пробел
        // unsigned char нужен потому что std::isspace ожидает беззнаковый тип
        // иначе на символах с кодом > 127 будет undefined behavior
        const auto isNotSpace = [](unsigned char ch)
        {
            return std::isspace(ch) == 0;
        };

        // find_if из ranges ищет первый символ слева, который не пробел
        auto firstNonSpace = std::ranges::find_if(source, isNotSpace);

        // rbegin/rend позволяют идти по строке справа налево
        // .base() конвертирует обратный итератор обратно в обычный
        auto lastNonSpace  = std::find_if(source.rbegin(), source.rend(), isNotSpace).base();

        std::string result;

        // если firstNonSpace < lastNonSpace — значит в строке есть непробельные символы
        if (firstNonSpace < lastNonSpace)
        {
            // assign заполняет result символами из диапазона [firstNonSpace, lastNonSpace)
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

        // istringstream — это поток, который читает из строки как из файла
        // удобно тем, что оператор >> автоматически пропускает пробелы между словами
        std::istringstream stream(line);
        std::string token;

        // читаем слова одно за другим пока поток не кончится
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

        // если число отрицательное, пропускаем знак минус
        if (!token.empty() && token[0] == '-')
        {
            start = 1;
        }

        // ищем точку начиная с позиции start (после знака, если был)
        // find возвращает string::npos если точка не найдена
        std::size_t dotPosition = token.find('.', start);

        // если точки нет — конец строки и есть конец целой части
        // если точка есть — то до неё
        const std::size_t end = (dotPosition == std::string::npos)
                                ? token.size() : dotPosition;

        // количество символов между start и end — это и есть цифры целой части
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
        // ищем точку в строке, ищем с начала
        std::size_t dotPosition = token.find('.');
        int result = 0;

        if (dotPosition != std::string::npos)
        {
            // всё что после точки — дробная часть
            // -1 потому что сама точка в счёт не идёт
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
        // ostringstream — поток записи в строку, работает как cout но в память
        std::ostringstream stream;

        // fixed означает: выводить число в формате "1.500", а не "1.5e0"
        stream.setf(std::ios::fixed);

        // precision(3) — ровно три знака после запятой
        stream.precision(3);
        stream << absValue;
        std::string text = stream.str();

        // убираем лишние нули в конце: "1.500" → "1.5"
        // find_last_not_of ищет последний символ, который НЕ является нулём
        std::size_t lastNonZero = text.find_last_not_of('0');
        if (lastNonZero != std::string::npos)
        {
            // erase удаляет всё после последнего не-нуля
            text.erase(lastNonZero + 1);
        }

        // если после удаления нулей осталась "2." — убираем точку тоже
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
        // начинаем с предположения что лучшая строка — это startRow
        int pivotRow = startRow;

        // fabs берёт абсолютное значение (модуль числа) из <cmath>
        double pivotMagnitude = std::fabs(matrix.data[startRow][col]);

        // перебираем строки ниже и ищем ту, где элемент больше по модулю
        for (int row = startRow + 1; row < matrix.rows; ++row)
        {
            const double currentMagnitude = std::fabs(matrix.data[row][col]);

            // нашли строку с большим по модулю элементом — запоминаем её
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
        // сохраняем ведущий элемент, на который будем делить всю строку
        const double pivotValue = matrix.data[rowIndex][pivotCol];

        // делим каждый элемент строки на pivotValue
        // после этого элемент в pivotCol станет ровно 1.0
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
            // ведущую строку трогать не нужно — пропускаем её
            if (row != pivotRow)
            {
                // берём коэффициент из текущей строки в столбце ведущего элемента
                // именно на этот коэффициент умножим ведущую строку перед вычитанием
                const double factor = matrix.data[row][pivotCol];

                // вычитаем из каждого элемента текущей строки
                // соответствующий элемент ведущей строки умноженный на factor
                // благодаря этому элемент в столбце pivotCol станет нулём
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
        // последний столбец матрицы — это свободные члены (правая часть уравнений)
        // поэтому число неизвестных на единицу меньше числа столбцов
        const int unknownsCount = matrix.cols - 1;

        bool foundContradiction = false;

        // считаем сколько строк содержат хотя бы один ненулевой коэффициент
        int nonZeroRows = 0;

        for (int row = 0; row < matrix.rows; ++row)
        {
            // пока не встретили ненулевой коэффициент — считаем строку нулевой
            bool allCoefficientsZero = true;

            for (int col = 0; col < unknownsCount; ++col)
            {
                // сравниваем с порогом а не с нулём чтобы не споткнуться о погрешность
                if (std::fabs(matrix.data[row][col]) > kNumericTolerance)
                {
                    allCoefficientsZero = false;
                }
            }

            const double freeMember = matrix.data[row][unknownsCount];

            // строка "0 0 ... 0 | b" где b != 0 — это 0 = b, то есть противоречие
            // такая система решений не имеет
            if (allCoefficientsZero && std::fabs(freeMember) > kNumericTolerance)
            {
                foundContradiction = true;
            }

            if (!allCoefficientsZero)
            {
                ++nonZeroRows;
            }
        }

        // начинаем с оптимистичного предположения — решение единственное
        SolutionType result = SolutionType::uniqueSolution;
        if (foundContradiction)
        {
            result = SolutionType::noSolutions;
        }
        else if (nonZeroRows < unknownsCount)
        {
            // ненулевых строк меньше чем неизвестных — у системы бесконечно много решений
            // это означает что часть переменных является свободными
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

        // создаём вектор для ответов, изначально заполненный нулями
        std::vector<double> solution(unknownsCount, 0.0);

        for (int unknownIndex = 0; unknownIndex < unknownsCount; ++unknownIndex)
        {
            for (int row = 0; row < matrix.rows; ++row)
            {
                // ищем строку, где в столбце unknownIndex стоит единица
                // это и есть строка, которая "отвечает" за данное неизвестное
                if (std::fabs(matrix.data[row][unknownIndex] - 1.0) < kNumericTolerance)
                {
                    // свободный член этой строки и есть значение переменной x_i
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
            // emplace_back создаёт объект Error прямо внутри вектора не копируя
            // это чуть эффективнее чем push_back с уже созданным объектом
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
                // stod — "string to double", стандартная функция из <string>
                // преобразует строку типа "3.14" в число 3.14
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
        // по умолчанию не добавляем строку пока не убедимся что всё ок
        addRowToMatrix = false;
        const int rawLength = static_cast<int>(rawLine.size());

        if (rawLength > kMaxLineLength)
        {
            // передаём реальную длину строки чтобы пользователь видел насколько она длинная
            errors.emplace_back(ErrorType::lineTooLong, "",
                                rowNumber, -1, rawLength);
        }
        else
        {
            // убираем пробелы по краям перед проверкой на пустоту
            const std::string trimmed = trimWhitespace(rawLine);

            if (trimmed.empty())
            {
                // строка либо пустая, либо содержала только пробелы
                errors.emplace_back(ErrorType::emptyLine, "", rowNumber);
            }
            else
            {
                // разбиваем строку на отдельные числа
                const std::vector<std::string> tokens = splitIntoTokens(trimmed);

                for (std::size_t tokenIndex = 0;
                     tokenIndex < tokens.size();
                     ++tokenIndex)
                {
                    // +1 потому что нумерация столбцов для пользователя начинается с 1
                    processToken(tokens[tokenIndex],
                                 rowNumber,
                                 static_cast<int>(tokenIndex) + 1,
                                 rowValues,
                                 errors);
                }

                // добавляем строку в матрицу только если все токены успешно разобраны
                // если размеры совпадают — значит ни один processToken ошибку не записал
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
            // файл был пустой или все строки оказались некорректными
            errors.emplace_back(ErrorType::emptyFile);
        }
        else
        {
            // берём количество столбцов первой строки как эталон
            const std::size_t firstRowSize = matrix.data[0].size();
            bool allRowsSameSize = true;

            // проходим по всем строкам и проверяем что столбцов везде поровну
            for (const auto& row : matrix.data)
            {
                if (row.size() != firstRowSize)
                {
                    allRowsSameSize = false;
                }
            }

            if (!allRowsSameSize)
            {
                // строки с разным числом элементов — матрицу из них не построить
                errors.emplace_back(ErrorType::unequalColumns);
            }
            else
            {
                // проверяем что уравнений не слишком мало и не слишком много
                if (matrix.rows < kMinEquations)
                {
                    errors.emplace_back(ErrorType::tooFewRows);
                }
                if (matrix.rows > kMaxEquations)
                {
                    errors.emplace_back(ErrorType::tooManyRows);
                }

                // последний столбец — свободные члены, поэтому неизвестных на 1 меньше
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

    // position — это курсор, который мы двигаем по строке слева направо
    std::size_t position = 0;
    const std::size_t length = token.size();

    if (length == 0)
    {
        result = false;
    }

    // знак минус допустим только в самом начале и только один
    if (result && position < length && token[position] == '-')
    {
        ++position;
    }

    // целая часть обязательна — хотя бы одна цифра должна быть
    const std::size_t integerStart = position;

    // isdigit проверяет что символ является цифрой 0-9
    // cast в unsigned char нужен чтобы избежать проблем с расширенными символами
    while (result && position < length
           && std::isdigit(static_cast<unsigned char>(token[position])))
    {
        ++position;
    }

    // если позиция не сдвинулась — значит цифр не было вовсе (или была только "-")
    if (result && position == integerStart)
    {
        result = false;
    }

    // проверяем необязательную дробную часть — точка + хотя бы одна цифра
    if (result && position < length && token[position] == '.')
    {
        ++position;
        const std::size_t fractionStart = position;

        while (position < length
               && std::isdigit(static_cast<unsigned char>(token[position])))
        {
            ++position;
        }

        // точка есть, а цифр после неё нет ("5." — это недопустимо)
        if (position == fractionStart)
        {
            result = false;
        }
    }

    // если курсор не дошёл до конца — значит остались какие-то лишние символы
    // например буквы, второй знак, скобки — всё это делает строку невалидной
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
    // round возвращает ближайшее целое число в виде double
    const double rounded = std::round(value);
    std::string result;

    // проверяем насколько value отличается от ближайшего целого
    if (std::fabs(value - rounded) < kNumericTolerance)
    {
        // число фактически целое — выводим без дробной части
        // приводим к long long чтобы вывести без ".0"
        if (static_cast<long long>(rounded) == 0)
        {
            // особый случай: -0.0 должен выводиться как "0" а не "-0"
            result = "0";
        }
        else
        {
            // ostringstream сам подбирает формат для целого числа
            std::ostringstream stream;
            stream << static_cast<long long>(rounded);
            result = stream.str();
        }
    }
    else
    {
        // число дробное — форматируем его абсолютное значение
        // знак добавим вручную после, если число отрицательное
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
    // очищаем выходной вектор на случай если там что-то было от прошлого вызова
    solution.clear();

    // количество неизвестных = количество столбцов минус один (последний — свободные члены)
    const int unknownsCount = matrix.cols - 1;

    // leadRow — это индекс строки, в которую мы "ставим" следующий ведущий элемент
    // он движется вниз по мере того как мы обрабатываем каждый столбец
    int leadRow = 0;

    for (int col = 0; col < unknownsCount && leadRow < matrix.rows; ++col)
    {
        // выбираем строку с наибольшим по модулю элементом в текущем столбце
        // это называется частичный выбор ведущего элемента (partial pivoting)
        // он нужен чтобы уменьшить накопление погрешности при делении
        const int pivotRow = findPivotRow(matrix, col, leadRow);
        const double pivotValue = std::fabs(matrix.data[pivotRow][col]);

        // если лучший элемент в столбце практически равен нулю — столбец вырожден
        // просто пропускаем его и идём к следующему
        if (pivotValue >= kNumericTolerance)
        {
            // меняем местами строку с ведущим элементом и текущую ведущую строку
            // swap из стандартной библиотеки меняет местами два объекта
            std::swap(matrix.data[leadRow], matrix.data[pivotRow]);

            // делим всю ведущую строку на её ведущий элемент, чтобы он стал 1
            normalizeRow(matrix, leadRow, col);

            // обнуляем все остальные элементы в этом столбце (выше и ниже ведущей строки)
            eliminateColumn(matrix, leadRow, col);

            // переходим к следующей ведущей строке
            ++leadRow;
        }
    }

    // после прямого хода анализируем что получилось — есть ли решение и сколько их
    const SolutionType solutionType = classifySolution(matrix);

    if (solutionType == SolutionType::uniqueSolution)
    {
        // решение единственное — извлекаем значения переменных из диагональной матрицы
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
    // сбрасываем матрицу и список ошибок перед началом чтения
    matrix = Matrix();
    errors.clear();

    // ifstream — поток чтения из файла, открывает файл по переданному пути
    std::ifstream stream(inputPath);
    bool result = true;

    if (!stream.is_open())
    {
        // файл не существует, нет прав доступа или путь некорректный
        errors.emplace_back(ErrorType::inputFileNotExist);
        result = false;
    }
    else
    {
        std::string rawLine;
        int rowNumber = 0;

        // getline читает строку целиком включая пробелы, останавливаясь на '\n'
        // возвращает ссылку на поток, которая становится false когда файл кончился
        while (std::getline(stream, rawLine))
        {
            ++rowNumber;
            std::vector<double> rowValues;
            bool addRow = false;

            // разбираем строку и собираем числа или ошибки
            processLine(rawLine, rowNumber, rowValues, errors, addRow);

            if (addRow)
            {
                // строка была корректной — добавляем её числа как новую строку матрицы
                matrix.data.push_back(rowValues);
            }
        }

        // заполняем поля размерности по фактическому содержимому матрицы
        matrix.rows = static_cast<int>(matrix.data.size());
        matrix.cols = (matrix.rows > 0)
                      ? static_cast<int>(matrix.data[0].size())
                      : 0;

        // проверку размерности делаем только если при разборе строк не было ошибок
        // иначе сообщения об ошибках начнут дублироваться и перекрываться
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
    // ofstream — поток записи в файл, создаёт файл если его нет
    // или перезаписывает если уже существует
    std::ofstream stream(outputPath);
    bool result = true;

    if (!stream.is_open())
    {
        // не удалось открыть файл — скорее всего нет прав на запись или неверный путь
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
                // выводим все значения через пробел
                // перед первым значением пробел не ставим
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