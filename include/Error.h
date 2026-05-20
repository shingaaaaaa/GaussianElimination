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

class Error
{
public:
    /// Тип ошибки.
    ErrorType type;

    /// Некорректное значение, если оно есть (для диагностических сообщений).
    std::string badValue;

    /// Номер строки в файле (с 1); -1, если ошибка не привязана к строке.
    int row;

    /// Номер столбца (позиция лексемы) в строке (с 1); -1, если не применимо.
    int col;

    /// Фактическая длина строки в символах (для lineTooLong); -1, если не применимо.
    int lineSize;

    /// Конструктор с инициализацией всех полей.
    Error(ErrorType errorType,
          const std::string& badValueStr = "",
          int rowNumber = -1,
          int colNumber = -1,
          int lineLength = -1);

    std::string generateErrorMessage() const;
};

#endif // SLU_SOLVER_ERROR_H
