/**
 * @file main.cpp
 * @brief Точка входа программы решения СЛАУ методом Гаусса.
 *
 * Программа принимает два аргумента командной строки:
 *   - входной файл с расширенной матрицей коэффициентов;
 *   - выходной файл для записи результата.
 *
 * Использование:
 * @code
 *   slu_solver <входной_файл> <выходной_файл>
 * @endcode
 *
 * Коды возврата:
 *   - 0 — успешное завершение;
 *   - 1 — ошибка (некорректные аргументы, ошибки во входных данных
 *           или невозможность создать выходной файл).
 */

#include <iostream>
#include <string>
#include <vector>

#include "../include/Error.h"
#include "../include/Matrix.h"
#include "../include/Solver.h"

namespace
{
    /**
     * @brief Выводит в stderr справочное сообщение о способе запуска.
     * @param programName Имя исполняемого файла (argv[0]).
     */
    void printUsage(const char* programName)
    {
        std::cerr << "Использование: "
                  << programName
                  << " <входной_файл> <выходной_файл>"
                  << std::endl;
    }

    /**
     * @brief Выводит в stderr все накопленные ошибки построчно.
     *
     *        Вызывается после неудачного readMatrix, чтобы пользователь
     *        получил полный список проблем за один запуск программы.
     *
     * @param errors Вектор ошибок, собранный при разборе входного файла.
     */
    void printErrors(const std::vector<Error>& errors)
    {
        for (const Error& error : errors)
        {
            std::cerr << error.generateErrorMessage() << std::endl;
        }
    }

} // namespace

/**
 * @brief Точка входа.
 *
 *        Алгоритм работы:
 *        1. Проверяем количество аргументов — ровно два (входной и выходной файл).
 *        2. Читаем и валидируем матрицу из входного файла (readMatrix).
 *        3. Если матрица корректна — решаем систему (gaussianElimination).
 *        4. Записываем результат в выходной файл (writeResult).
 *        5. При любой ошибке выводим сообщение в stderr и возвращаем код 1.
 *
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов: argv[1] — входной файл, argv[2] — выходной.
 * @return 0 при успехе, 1 при ошибке.
 */
int main(int argc, char* argv[])
{
    int exitCode = 0;

    // Проверяем что передано ровно два аргумента.
    if (argc != 3)
    {
        printUsage((argc > 0) ? argv[0] : "slu_solver");
        exitCode = 1;
    }
    else
    {
        const std::string inputPath  = argv[1];
        const std::string outputPath = argv[2];

        Matrix matrix;
        std::vector<Error> errors;

        // Читаем матрицу; при ошибках выводим их все и завершаемся.
        const bool readOk = readMatrix(inputPath, matrix, errors);
        if (!readOk)
        {
            printErrors(errors);
            exitCode = 1;
        }
        else
        {
            // Решаем систему методом Гаусса.
            std::vector<double> solution;
            const SolutionType solutionType = gaussianElimination(matrix, solution);

            // Записываем результат; при неудаче сообщаем об ошибке записи.
            const bool writeOk = writeResult(outputPath, solutionType, solution);
            if (!writeOk)
            {
                Error writeError(ErrorType::outputFileCreateFail);
                std::cerr << writeError.generateErrorMessage() << std::endl;
                exitCode = 1;
            }
        }
    }

    return exitCode;
}