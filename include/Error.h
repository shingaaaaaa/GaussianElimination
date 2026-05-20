#ifndef SLU_SOLVER_ERROR_H
#define SLU_SOLVER_ERROR_H

#include <string>

/// Тип ошибки, обнаруженной при чтении или валидации входных данных.
enum class ErrorType
{
    inputFileNotExist,        ///< Входной файл не найден / нет доступа.
    outputFileCreateFail,     ///< Не удаётся создать выходной файл.
    emptyFile,                ///< Входной файл пуст.
    unequalColumns,           ///< В строках разное количество столбцов.
    tooFewRows,               ///< Количество уравнений меньше 2.
    tooManyRows,              ///< Количество уравнений превышает 10.
    tooFewCols,               ///< Количество неизвестных меньше 2.
    tooManyCols,              ///< Количество неизвестных превышает 10.
    nonNumericValue,          ///< Лексема не является действительным числом.
    tooManyDecimalPlaces,     ///< Число содержит более 3 знаков после запятой.
    integerPartTooLong,       ///< Целая часть числа превышает 15 цифр.
    emptyLine,                ///< Файл содержит пустую строку.
    lineTooLong               ///< Длина строки превышает 250 символов.
};

#endif // SLU_SOLVER_ERROR_H