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
        // TODO: реализовать чтение матрицы, решение и запись результата
        std::cerr << "Логика решения ещё не реализована." << std::endl;
        exitCode = 1;
    }
    return exitCode;
}