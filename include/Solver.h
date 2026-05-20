#ifndef SLU_SOLVER_SOLVER_H
#define SLU_SOLVER_SOLVER_H

#include <string>
#include <vector>

#include "Error.h"
#include "Matrix.h"

enum class SolutionType
{
    uniqueSolution,           ///< Существует единственное решение.
    noSolutions,              ///< Решений нет (несовместная система).
    infinitelyManySolutions   ///< Существует бесконечно много решений.
};

SolutionType gaussianElimination(Matrix& matrix,
                                 std::vector<double>& solution);


bool isValidNumber(const std::string& token);

std::string formatNumber(double value);

bool readMatrix(const std::string& inputPath,
                Matrix& matrix,
                std::vector<Error>& errors);

bool writeResult(const std::string& outputPath,
                 SolutionType type,
                 const std::vector<double>& solution);

#endif // SLU_SOLVER_SOLVER_H
