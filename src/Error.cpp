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

namespace
{
    /// Формирует базовое сообщение для каждого типа ошибки.
    std::string buildBaseMessage(ErrorType type, const std::string& badValue)
    {
        std::string message;
        switch (type)
        {
            case ErrorType::inputFileNotExist:
                message = "Ошибка ввода: Не удалось открыть входной файл. "
                          "Проверьте его наличие и права доступа.";
                break;
            case ErrorType::outputFileCreateFail:
                message = "Ошибка вывода: Не удалось создать выходной файл. "
                          "Проверьте правильность пути и права на запись.";
                break;
            case ErrorType::emptyFile:
                message = "Ошибка формата: Входной файл пуст.";
                break;
            case ErrorType::unequalColumns:
                message = "Ошибка формата: Неодинаковое количество столбцов "
                          "в строках файла.";
                break;
            case ErrorType::emptyLine:
                message = "Ошибка формата: Файл содержит пустую строку.";
                break;
            case ErrorType::lineTooLong:
                message = "Ошибка формата: Длина строки превышает максимально "
                          "допустимую (250 символов).";
                break;
            case ErrorType::tooFewRows:
                message = "Ошибка ограничения: Количество уравнений должно "
                          "быть не меньше 2.";
                break;
            case ErrorType::tooManyRows:
                message = "Ошибка ограничения: Количество уравнений не должно "
                          "превышать 10.";
                break;
            case ErrorType::tooFewCols:
                message = "Ошибка ограничения: Количество неизвестных должно "
                          "быть не меньше 2.";
                break;
            case ErrorType::tooManyCols:
                message = "Ошибка ограничения: Количество неизвестных не должно "
                          "превышать 10.";
                break;
            case ErrorType::nonNumericValue:
                message = "Ошибка данных: Обнаружено нечисловое значение «"
                          + badValue + "».";
                break;
            case ErrorType::tooManyDecimalPlaces:
                message = "Ошибка данных: Число " + badValue
                          + " содержит более 3 знаков после запятой.";
                break;
            case ErrorType::integerPartTooLong:
                message = "Ошибка данных: Целая часть числа " + badValue
                          + " превышает 15 цифр.";
                break;
        }
        return message;
    }
    /// Возвращает true, если для данного типа ошибки требуется указывать позицию.
    bool errorTypeHasPosition(ErrorType type)
    {
        bool result = false;
        switch (type)
        {
            case ErrorType::emptyLine:
            case ErrorType::lineTooLong:
            case ErrorType::nonNumericValue:
            case ErrorType::tooManyDecimalPlaces:
            case ErrorType::integerPartTooLong:
                result = true;
                break;
            default:
                result = false;
                break;
        }
        return result;
    }
}