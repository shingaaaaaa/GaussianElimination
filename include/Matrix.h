#ifndef SLU_SOLVER_MATRIX_H
#define SLU_SOLVER_MATRIX_H

#include <vector>

class Matrix
{
public:
    /// Количество строк (число уравнений).
    int rows;

    /// Количество столбцов (число неизвестных + 1).
    int cols;

    /// Двумерный массив элементов расширенной матрицы.
    std::vector<std::vector<double>> data;

    /// Создаёт пустую матрицу с нулевыми размерностями.
    Matrix();
};

#endif // SLU_SOLVER_MATRIX_H
