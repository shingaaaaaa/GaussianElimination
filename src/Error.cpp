#include "../include/Error.h"
#include <sstream>

Error::Error(ErrorType errorType,
             const std::string& badValueStr,
             int rowNumber,
             int colNumber,
             int lineLength)
    : type(errorType),
      badValue(badValueStr),
      row(rowNumber),
      col(colNumber),
      lineSize(lineLength)
{
}

std::string Error::generateErrorMessage() const
{
    std::ostringstream oss;

    switch (type)
    {
        case ErrorType::inputFileNotExist:
            return "Ошибка ввода: Не удалось открыть входной файл. "
                   "Проверьте его наличие и права доступа.";
        case ErrorType::emptyFile:
            return "Ошибка формата: Входной файл пуст.";
        case ErrorType::tooFewRows:
            return "Ошибка ограничения: Количество уравнений должно быть не меньше 2.";
        case ErrorType::tooManyRows:
            return "Ошибка ограничения: Количество уравнений не должно превышать 10.";
        case ErrorType::emptyLine:
            oss << "Ошибка формата: Файл содержит пустую строку";
            break;
        case ErrorType::nonNumericValue:
            oss << "Ошибка данных: Обнаружено нечисловое значение «"
                << badValue << "»";
            break;
        case ErrorType::lineTooLong:
            oss << "Ошибка формата: Длина строки превышает максимально допустимую (250 символов)";
            if (lineSize != -1)
                oss << ", длина: " << lineSize << " символов";
            break;

        case ErrorType::unequalColumns:
            return "Ошибка формата: Неодинаковое количество столбцов в строках файла.";
        case ErrorType::outputFileCreateFail:
            return "Ошибка вывода: Не удалось создать выходной файл. "
                   "Проверьте правильность пути и права на запись.";
        default:
            return "";
    }

    if (row != -1)
        oss << " (строка: " << row;
    if (col != -1)
        oss << ", столбец: " << col;
    if (row != -1 || col != -1)
        oss << ")";

    return oss.str();
}