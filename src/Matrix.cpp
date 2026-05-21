/**
* @file Matrix.cpp
 * @brief Реализация конструктора класса Matrix.
 */

#include "../include/Matrix.h"

/**
 * @brief Создаёт пустую матрицу с нулевыми размерностями.
 *
 *        После создания rows == 0, cols == 0, data пуста.
 *        Размерности заполняются функцией readMatrix после
 *        успешного разбора входного файла.
 */
Matrix::Matrix()
    : rows(0),
      cols(0),
      data()
{
}