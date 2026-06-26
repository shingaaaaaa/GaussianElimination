/**
 * @file Error.cpp
 * @brief Реализация класса Error: конструктор и генерация
 *        диагностических сообщений для каждого типа ошибки.
 */

#include "../include/Error.h"

#include <sstream>

// Конструктор

/**
 * @brief Инициализирует объект ошибки всеми полями.
 *
 *        Поля row, col и lineSize по умолчанию равны -1, что означает
 *        «позиция неизвестна» — в этом случае суффикс с позицией
 *        в сообщение не добавляется.
 *
 * @param errorType   Тип обнаруженной ошибки.
 * @param badValueStr Некорректное значение (пустая строка, если неприменимо).
 * @param rowNumber   Номер строки в файле (с 1) или -1.
 * @param colNumber   Номер столбца (позиция токена) или -1.
 * @param lineLength  Фактическая длина строки (для lineTooLong) или -1.
 */
Error::Error(ErrorType errorType,
             const std::string& badValueStr,
             int rowNumber,
             int colNumber,
             int lineLength)
    // список инициализации присваивает значения полям ещё до входа в тело конструктора
    // это эффективнее чем присваивание внутри тела, особенно для строк
    : type(errorType),
      badValue(badValueStr),
      row(rowNumber),
      col(colNumber),
      lineSize(lineLength)
{
}

// Вспомогательные функции
namespace
{
    /**
     * @brief Формирует суффикс с указанием позиции в файле.
     *
     *        Выводит только те поля, значения которых отличны от -1.
     *        Примеры:
     *        - " (строка: 3, столбец: 2)"
     *        - " (строка: 5, длина строки: 280 символов)"
     *        - "" — если все три поля равны -1.
     *
     * @param row      Номер строки или -1.
     * @param col      Номер столбца или -1.
     * @param lineSize Длина строки или -1.
     * @return Строка суффикса или пустая строка.
     */
    std::string buildPositionSuffix(int row, int col, int lineSize)
    {
        // проверяем есть ли хоть какая-то позиционная информация для отображения
        const bool hasAnyPosition = (row != -1) || (col != -1) || (lineSize != -1);
        std::string suffix;

        if (hasAnyPosition)
        {
            // собираем суффикс в потоке — удобнее чем конкатенировать строки вручную
            std::ostringstream stream;
            stream << " (";
            // флаг чтобы понимать нужна ли запятая перед следующим полем
            bool needSeparator = false;

            if (row != -1)
            {
                stream << "строка: " << row;
                needSeparator = true;
            }

            if (col != -1)
            {
                if (needSeparator)
                {
                    stream << ", ";
                }
                stream << "столбец: " << col;
                needSeparator = true;
            }

            // длина строки нужна только для ошибки lineTooLong
            // чтобы пользователь видел на сколько именно строка превышает лимит
            if (lineSize != -1)
            {
                if (needSeparator)
                {
                    stream << ", ";
                }
                stream << "длина строки: " << lineSize << " символов";
            }

            stream << ")";
            suffix = stream.str();
        }

        return suffix;
    }

    /**
     * @brief Формирует основной текст сообщения для данного типа ошибки.
     *
     *        Сообщения разделены на три категории:
     *        - «Ошибка ввода/вывода» — проблемы с файлами;
     *        - «Ошибка формата»      — структура входных данных нарушена;
     *        - «Ошибка ограничения»  — выход за допустимые пределы спецификации;
     *        - «Ошибка данных»       — некорректные числовые значения.
     *
     * @param type     Тип ошибки.
     * @param badValue Некорректное значение (встраивается в сообщение если нужно).
     * @return Текст сообщения без суффикса позиции.
     */
    std::string buildBaseMessage(ErrorType type, const std::string& badValue)
    {
        std::string message;
        switch (type)
        {
            //  Ошибки файловой системы
            case ErrorType::inputFileNotExist:
                message = "Ошибка ввода: Не удалось открыть входной файл. "
                          "Проверьте его наличие и права доступа.";
                break;

            case ErrorType::outputFileCreateFail:
                message = "Ошибка вывода: Не удалось создать выходной файл. "
                          "Проверьте правильность пути и права на запись.";
                break;

            // Ошибки структуры файла
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

            // Ошибки ограничений спецификации
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

            //  Ошибки числовых значений
            case ErrorType::nonNumericValue:
                // встраиваем само значение в сообщение чтобы пользователь сразу видел что не так
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

    /**
     * @brief Определяет, нужно ли добавлять суффикс позиции к сообщению.
     *
     *        Позиция выводится для ошибок, привязанных к конкретному
     *        месту в файле: пустая строка, длинная строка, некорректный токен.
     *        Для ошибок уровня файла или ограничений позиция не нужна.
     *
     * @param type Тип ошибки.
     * @return true, если суффикс позиции должен быть добавлен.
     */
    bool errorTypeHasPosition(ErrorType type)
    {
        bool result = false;
        switch (type)
        {
            // эти ошибки всегда привязаны к конкретной строке или позиции в файле
            case ErrorType::emptyLine:
            case ErrorType::lineTooLong:
            case ErrorType::nonNumericValue:
            case ErrorType::tooManyDecimalPlaces:
            case ErrorType::integerPartTooLong:
                result = true;
                break;

            // все остальные ошибки относятся к файлу в целом — позиция не нужна
            default:
                result = false;
                break;
        }
        return result;
    }

} // namespace

//Публичные методы

/**
 * @brief Генерирует полное текстовое сообщение об ошибке.
 *
 *        Объединяет базовое сообщение с суффиксом позиции (строка/столбец),
 *        если тип ошибки предполагает привязку к конкретному месту файла.
 *
 * @return Строка, готовая для вывода в stderr.
 */
std::string Error::generateErrorMessage() const
{
    // формируем основной текст сообщения по типу ошибки
    std::string message = buildBaseMessage(type, badValue);

    // дописываем позицию только для тех типов ошибок, где это имеет смысл
    // например для "файл пустой" позиция ни к чему, а для "нечисловое значение" нужна
    if (errorTypeHasPosition(type))
    {
        message += buildPositionSuffix(row, col, lineSize);
    }

    return message;
}
