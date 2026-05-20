#include <iostream>
#include <string>
#include <vector>

#include "../include/Error.h"
#include "../include/Matrix.h"
#include "../include/Solver.h"

namespace
{
    /// Выводит в stderr справочное сообщение о способе запуска программы.
    void printUsage(const char* programName)
    {
        std::cerr << "Использование: "
                  << programName
                  << " <входной_файл> <выходной_файл>"
                  << std::endl;
    }
}

int main(int argc, char* argv[])
{
    int exitCode = 0;
    if (argc != 3)
    {
        printUsage((argc > 0) ? argv[0] : "slu_solver");
        exitCode = 1;
    }
    else
    {
        const std::string inputPath = argv[1];
        const std::string outputPath = argv[2];

        Matrix matrix;
        std::vector<Error> errors;

        const bool readOk = readMatrix(inputPath, matrix, errors);
        if (!readOk)
        {
            // Пока выводим только первое сообщение об ошибке (временное решение)
            if (!errors.empty())
                std::cerr << errors[0].generateErrorMessage() << std::endl;
            exitCode = 1;
        }
        else
        {
            std::vector<double> solution;
            const SolutionType solutionType = gaussianElimination(matrix, solution);
            const bool writeOk = writeResult(outputPath, solutionType, solution);
            if (!writeOk)
            {
                std::cerr << "Ошибка: невозможно создать выходной файл." << std::endl;
                exitCode = 1;
            }
        }
    }
    return exitCode;
}