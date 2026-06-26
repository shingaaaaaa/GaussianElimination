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
        // std::cerr — стандартный поток ошибок, он не буферизуется как cout
        // пишем сюда а не в cout чтобы сообщение об ошибке не смешивалось с нормальным выводом
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
        // range-based for — перебирает все элементы вектора по одному
        // const Error& означает что берём ссылку без копирования объекта
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
    // 0 означает успех, 1 — ошибку; начинаем оптимистично
    int exitCode = 0;

    // argc включает само имя программы, поэтому 3 аргумента = имя + входной + выходной
    if (argc != 3)
    {
        // если argc > 0 то argv[0] содержит имя программы, иначе используем заглушку
        printUsage((argc > 0) ? argv[0] : "slu_solver");
        exitCode = 1;
    }
    else
    {
        // преобразуем C-строки в std::string для удобной передачи в функции
        const std::string inputPath  = argv[1];
        const std::string outputPath = argv[2];

        Matrix matrix;
        std::vector<Error> errors;

        // пытаемся прочитать матрицу из файла
        // функция сама разбирает содержимое и проверяет корректность
        const bool readOk = readMatrix(inputPath, matrix, errors);
        if (!readOk)
        {
            // выводим все найденные ошибки разом — пользователь сразу видит все проблемы
            printErrors(errors);
            exitCode = 1;
        }
        else
        {
            // матрица корректна — запускаем метод Гаусса
            std::vector<double> solution;
            const SolutionType solutionType = gaussianElimination(matrix, solution);

            // записываем результат в выходной файл (решение или сообщение о его отсутствии)
            const bool writeOk = writeResult(outputPath, solutionType, solution);
            if (!writeOk)
            {
                // не удалось записать результат — создаём объект ошибки и выводим сообщение
                Error writeError(ErrorType::outputFileCreateFail);
                std::cerr << writeError.generateErrorMessage() << std::endl;
                exitCode = 1;
            }
        }
    }

    return exitCode;
}
