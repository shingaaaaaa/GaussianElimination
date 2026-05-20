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
    switch (type)
    {
        case ErrorType::inputFileNotExist:
            return "Ошибка ввода: Не удалось открыть входной файл. "
                   "Проверьте его наличие и права доступа.";
        default:
            return "";
    }
}