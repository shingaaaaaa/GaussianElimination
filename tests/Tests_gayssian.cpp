
#include <gtest/gtest.h>

#include <cmath>
#include <initializer_list>
#include <vector>

#include "../include/Solver.h"
#include "../include/Matrix.h"

static Matrix makeMatrix(int rows,
                         int cols,
                         std::initializer_list<double> values)
{
    Matrix matrix;
    matrix.rows = rows;
    matrix.cols = cols;

    matrix.data.assign(rows,
                        std::vector<double>(cols, 0.0));

    auto iterator = values.begin();

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            matrix.data[row][col] = *iterator;
            ++iterator;
        }
    }

    return matrix;
}

static void expectVectorsNear(
    const std::vector<double>& actual,
    const std::vector<double>& expected,
    double tolerance)
{
    ASSERT_EQ(actual.size(), expected.size());

    for (std::size_t index = 0;
         index < expected.size();
         ++index)
    {
        EXPECT_NEAR(actual[index],
                    expected[index],
                    tolerance);
    }
}

// Тест 1. Простая система 2x2 с единственным решением
// x1 + x2 = 4
// x1 + 2*x2 = 5
// Решение: x1 = 3, x2 = 1.

TEST(GaussianEliminationTest, Simple2x2Unique)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            1, 1, 4,
            1, 2, 5
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::uniqueSolution);

    expectVectorsNear(solution,
                      {3.0, 1.0},
                      1e-6);
}

// Тест 2. Квадратная 3x3 с единственным решением
// 2*x1 + 3*x2 +   x3 = 5
// 4*x1 +   x2 + 2*x3 = 6
// 3*x1 + 2*x2 + 3*x3 = 7
// Решение: x1 = 0.8, x2 = 0.8, x3 = 1.0.
TEST(GaussianEliminationTest, Square3x3Unique)
{
    Matrix matrix =
        makeMatrix(3, 4,
        {
            2, 3, 1, 5,
            4, 1, 2, 6,
            3, 2, 3, 7
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::uniqueSolution);

    expectVectorsNear(solution,
                      {0.8, 0.8, 1.0},
                      1e-6);
}

// Тест 3. Дробные коэффициенты
// 0.5*x1 + 1.5*x2 = 3
//   x1   + 2.5*x2 = 5
// Решение: x1 = 0, x2 = 2.
TEST(GaussianEliminationTest, FractionalUnique)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            0.5, 1.5, 3.0,
            1.0, 2.5, 5.0
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::uniqueSolution);

    expectVectorsNear(solution,
                      {0.0, 2.0},
                      1e-6);
}

// Тест 4. Несовместная система (нет решений)
// x1 + x2 = 4
// x1 + x2 = 5  (противоречие)

TEST(GaussianEliminationTest, NoSolutions)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            1, 1, 4,
            1, 1, 5
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::noSolutions);

    EXPECT_TRUE(solution.empty());
}


// Тест 6. Квадратная система с нулём в решении
// x1 + 2*x2 + 3*x3 = 1
//       x2         = 0
//                x3 = 0
// Решение: x1 = 1, x2 = 0, x3 = 0.

TEST(GaussianEliminationTest, UniqueWithZeros)
{
    Matrix matrix =
        makeMatrix(3, 4,
        {
            1, 2, 3, 1,
            0, 1, 0, 0,
            0, 0, 1, 0
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::uniqueSolution);

    expectVectorsNear(solution,
                      {1.0, 0.0, 0.0},
                      1e-6);
}


// Тест 8. Нулевой первый элемент: проверка частичного выбора
    // 0*x1 + x2 = 2
    //  x1 + x2 = 3
    // Алгоритм должен переставить строки и не делить на 0.
    // Решение: x1 = 1, x2 = 2.

TEST(GaussianEliminationTest, ZeroPivotSwap)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            0, 1, 2,
            1, 1, 3
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::uniqueSolution);

    expectVectorsNear(solution,
                      {1.0, 2.0},
                      1e-6);
}

// Тест 9. Решение содержит несколько нулей
    // Уже частично покрыт в тесте 6, здесь проверяем другую структуру.
    // 2*x1 + 0*x2 + 0*x3 = 4
    // 0*x1 + 3*x2 + 0*x3 = 0
    // 0*x1 + 0*x2 +   x3 = 5
    // Решение: x1 = 2, x2 = 0, x3 = 5.

TEST(GaussianEliminationTest, DiagonalWithZero)
{
    Matrix matrix =
        makeMatrix(3, 4,
        {
            2, 0, 0, 4,
            0, 3, 0, 0,
            0, 0, 1, 5
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::uniqueSolution);

    expectVectorsNear(solution,
                      {2.0, 0.0, 5.0},
                      1e-6);
}

// Тест 10. Дробные коэффициенты, требующие точности
    // 1.333*x1 + 2.667*x2 = 4
    // 2.667*x1 + 5.333*x2 = 8
    // Близко к вырожденной, но определитель ненулевой.

TEST(GaussianEliminationTest,
     FractionalCloseToDegenerate)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            1.333, 2.667, 4.0,
            2.667, 5.333, 8.0
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::uniqueSolution);

    expectVectorsNear(solution,
                      {1.0, 1.0},
                      1e-2);
}

// Тест 11. Отрицательное решение
    //  x1 + x2 = -1
    //  x1 - x2 =  3
    // Решение: x1 = 1, x2 = -2.

TEST(GaussianEliminationTest,
     NegativeSolution)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            1,  1, -1,
            1, -1,  3
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::uniqueSolution);

    expectVectorsNear(solution,
                      {1.0, -2.0},
                      1e-6);
}
// Тест 12. Проверка частичного выбора ведущего элемента
// 0.001*x1 + 1*x2 = 2
// 10*x1   + 2*x2 = 14
// Алгоритм должен выбрать ведущий элемент 10 (вторую строку),
// а не 0.001, чтобы избежать потери точности.
// Решение: x1 ≈ 1, x2 ≈ 1.999

TEST(GaussianEliminationTest,
     PartialPivotingMaximumAbsolute)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            0.001, 1,  2,
            10,    2, 14
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::uniqueSolution);

    // x1 ≈ 1, x2 ≈ 1.999
    expectVectorsNear(solution,
                      {1.0, 1.999},
                      1e-3);
}


// Тест 15. Противоречие после исключения
// 1*x1 + 2*x2 = 3
// 2*x1 + 4*x2 = 7
// Второе уравнение: 2*(x1+2x2) = 2*3 = 6, но правая часть = 7 → противоречие.
// После исключения получится строка [0, 0 | 1], что означает отсутствие решений.

TEST(GaussianEliminationTest,
     ContradictionAfterElimination)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            1, 2, 3,
            2, 4, 7
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::noSolutions);

    EXPECT_TRUE(solution.empty());
}

// Тест 16. Переопределённая система с единственным решением
// 3 уравнения, 2 неизвестных (переопределённая система)
// x1 + x2 = 2
// 2x1 + x2 = 3
// 3x1 + 2x2 = 5
// Система совместна и имеет единственное решение: x1 = 1, x2 = 1.
// (Третье уравнение является суммой первых двух)

TEST(GaussianEliminationTest,
     OverdeterminedUniqueSolution)
{
    Matrix matrix =
        makeMatrix(3, 3,
        {
            1, 1, 2,
            2, 1, 3,
            3, 2, 5
        });

    std::vector<double> solution;

    SolutionType type =
        gaussianElimination(matrix, solution);

    EXPECT_EQ(type,
              SolutionType::uniqueSolution);

    expectVectorsNear(solution,
                      {1.0, 1.0},
                      1e-6);
}
// Тест 17. Переопределённая несовместная система
// x₁ + x₂ = 2
// 2x₁ + 2x₂ = 4  (удвоенное первое, противоречия нет)
// x₁ + x₂ = 3    (противоречие с первым: 2 ≠ 3)
// Система не имеет решений (noSolutions)

TEST(GaussianEliminationTest, OverdeterminedInconsistent)
{
    Matrix matrix =
        makeMatrix(3, 3,
        {
            1, 1, 2,
            2, 2, 4,
            1, 1, 3
        });

    std::vector<double> solution;
    SolutionType type = gaussianElimination(matrix, solution);

    // Программа возвращает noSolutions
    EXPECT_EQ(type, SolutionType::noSolutions);
    EXPECT_TRUE(solution.empty());
}

// Тест 18. Система с отрицательными коэффициентами
// -1*x1 + 2*x2 = 3
//  3*x1 - 1*x2 = 2
// Решение: x1 = 1.4, x2 = 2.2
// Проверка корректной работы с отрицательными числами.

TEST(GaussianEliminationTest, NegativeCoefficientsSystem)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            -1, 2, 3,
             3, -1, 2
        });

    std::vector<double> solution;
    SolutionType type = gaussianElimination(matrix, solution);

    EXPECT_EQ(type, SolutionType::uniqueSolution);
    expectVectorsNear(solution, {1.4, 2.2}, 1e-6);
}

// Тест 19. Система с отрицательными дробными коэффициентами
// -0.5*x1 + 1.5*x2 = 2.5
//  1.5*x1 - 0.5*x2 = 0.5
// Решение: x1 = 1, x2 = 2
// Проверка работы с дробными отрицательными коэффициентами.

TEST(GaussianEliminationTest, NegativeFractionalSystem)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            -0.5, 1.5, 2.5,
             1.5, -0.5, 0.5
        });

    std::vector<double> solution;
    SolutionType type = gaussianElimination(matrix, solution);

    EXPECT_EQ(type, SolutionType::uniqueSolution);
    expectVectorsNear(solution, {1.0, 2.0}, 1e-6);
}

// Тест 20. Решение из отрицательных значений
//  x1 +  x2 = -3
// 2x1 -  x2 = -3
// Решение: x1 = -2, x2 = -1
// Проверка корректности получения отрицательных результатов.

TEST(GaussianEliminationTest, NegativeResultSolution)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            1,  1, -3,
            2, -1, -3
        });

    std::vector<double> solution;
    SolutionType type = gaussianElimination(matrix, solution);

    EXPECT_EQ(type, SolutionType::uniqueSolution);
    expectVectorsNear(solution, {-2.0, -1.0}, 1e-6);
}


// Тест 22. Почти нулевой ведущий элемент (порог 1e-9)
// 0.0000000001*x1 + 1*x2 = 2
// 1*x1           + 1*x2 = 3
// Решение: x1 = 1, x2 = 2
// Проверка, что алгоритм не делит на почти ноль.
// Частичный выбор должен выбрать строку с большим элементом (1 вместо 1e-10).

TEST(GaussianEliminationTest, AlmostZeroPivotThreshold)
{
    Matrix matrix =
        makeMatrix(2, 3,
        {
            0.0000000001, 1, 2,
            1,             1, 3
        });

    std::vector<double> solution;
    SolutionType type = gaussianElimination(matrix, solution);

    EXPECT_EQ(type, SolutionType::uniqueSolution);
    expectVectorsNear(solution, {1.0, 2.0}, 1e-6);
}

// Тест 23. Граничный размер 10×10
// Диагональная матрица 10×10 с известным решением:
// x1 = 1, x2 = 2, x3 = 3, ..., x10 = 10
// Проверка работоспособности на максимальном допустимом размере.
// Каждое i-е уравнение: xi = i

TEST(GaussianEliminationTest, BoundarySize10x10)
{
    // Диагональная матрица 10×10
    // 1 0 0 ... 0 | 1
    // 0 1 0 ... 0 | 2
    // 0 0 1 ... 0 | 3
    // ...
    // 0 0 0 ... 1 | 10

    Matrix matrix;
    matrix.rows = 10;
    matrix.cols = 11;
    matrix.data.assign(10, std::vector<double>(11, 0.0));

    std::vector<double> expected(10);
    for (int i = 0; i < 10; ++i)
    {
        matrix.data[i][i] = 1.0;
        matrix.data[i][10] = static_cast<double>(i + 1);
        expected[i] = static_cast<double>(i + 1);
    }

    std::vector<double> solution;
    SolutionType type = gaussianElimination(matrix, solution);

    EXPECT_EQ(type, SolutionType::uniqueSolution);
    EXPECT_EQ(solution.size(), 10);
    expectVectorsNear(solution, expected, 1e-6);
}

// Тест 24. Система, где требуется несколько перестановок строк
// 0*x1 + 0*x2 + 1*x3 = 3
// 0*x1 + 2*x2 + 1*x3 = 4
// 1*x1 + 1*x2 + 1*x3 = 6
// Решение: x1 = 2.5, x2 = 0.5, x3 = 3
// Алгоритм должен выполнить две перестановки строк:
// 1) Поменять строку 1 с строкой 3 (ведущий элемент в 1-м столбце)
// 2) Поменять строку 2 с строкой 3 (ведущий элемент во 2-м столбце)
// Проверка корректности нескольких перестановок строк.

TEST(GaussianEliminationTest, MultipleRowPermutations)
{
    Matrix matrix =
        makeMatrix(3, 4,
        {
            0, 0, 1, 3,
            0, 2, 1, 4,
            1, 1, 1, 6
        });

    std::vector<double> solution;
    SolutionType type = gaussianElimination(matrix, solution);

    EXPECT_EQ(type, SolutionType::uniqueSolution);
    expectVectorsNear(solution, {2.5, 0.5, 3.0}, 1e-6);
}

// Тест 25. Диагональная система
// 2*x1 + 0*x2 + 0*x3 = 4
// 0*x1 + 3*x2 + 0*x3 = 9
// 0*x1 + 0*x2 + 5*x3 = 10
// Решение: x1 = 2, x2 = 3, x3 = 2

TEST(GaussianEliminationTest, DiagonalSystemTest)
{
    Matrix matrix =
        makeMatrix(3, 4,
        {
            2, 0, 0, 4,
            0, 3, 0, 9,
            0, 0, 5, 10
        });

    std::vector<double> solution;
    SolutionType type = gaussianElimination(matrix, solution);

    EXPECT_EQ(type, SolutionType::uniqueSolution);
    expectVectorsNear(solution, {2.0, 3.0, 2.0}, 1e-6);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc,
                              argv);

    return RUN_ALL_TESTS();
}