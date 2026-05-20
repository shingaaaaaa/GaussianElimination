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

    /// Выводит в stderr все накопленные ошибки.
    void printErrors(const std::vector<Error>& errors)
    {
        for (const Error& error : errors)
        {
            std::cerr << error.generateErrorMessage() << std::endl;
        }
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
            printErrors(errors);
            exitCode = 1;
        }
        else
        {
            std::vector<double> solution;
            const SolutionType solutionType = gaussianElimination(matrix, solution);
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
