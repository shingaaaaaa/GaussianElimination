# Программа для решения СЛАУ

Реализация по внешней и внутренней спецификациям. Решает систему линейных
алгебраических уравнений методом Гаусса с частичным выбором ведущего элемента.

## Структура

```
GaussianElimination/
├── include/
│   ├── Error.h               # класс ошибок и их типы
│   ├── Matrix.h              # расширенная матрица коэффициентов
│   └── Solver.h              # интерфейс: gaussianElimination, readMatrix,
│                             # writeResult, isValidNumber, formatNumber
├── src/
│   ├── main.cpp              # точка входа (app)
│   ├── Error.cpp
│   ├── Matrix.cpp
│   ├── Solver.cpp
│   └── CMakeLists.txt        # цели: core (статическая библиотека), app
├── tests/
│   ├── Tests_gayssian.cpp           # тесты gaussianElimination
│   ├── run_tests.sh                 # интеграционные батарейные тесты
|   ├── run_tests.bat                # интеграционные батарейные тесты
│   └── CMakeLists.txt               # цель: unit_tests
├── docs/
│   └── Doxyfile.in           # шаблон конфигурации Doxygen
└── CMakeLists.txt            # корневой CMake (C++20, опция BUILD_DOCS)
```

## Требования

| Инструмент | Минимальная версия |
|---|---|
| CMake | 3.20 |
| Компилятор | C++20 (GCC 10+, Clang 13+, MSVC 2019+) |
| Doxygen | любая *(опционально)* |

Доступ к интернету во время сборки нужен только при первом запуске — CMake
автоматически скачает GoogleTest через `FetchContent`.

## Сборка

```bash
cmake -S . -B build
cmake --build build
```

Для Release-сборки:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Отключить генерацию документации:

```bash
cmake -S . -B build -DBUILD_DOCS=OFF
```

## Запуск

```bash
./build/src/app <входной_файл> <выходной_файл>
```

Коды возврата: `0` — успех, `1` — ошибка (неверные аргументы, некорректный
файл или невозможность создать выходной файл).

**Формат входного файла** — расширенная матрица, строки разделены
переносами, числа — пробелами или табуляцией:

```
2.0 1.0 -1.0 8.0
-3.0 -1.0 2.0 -11.123
-2.0 1.0 2.0 -3.0
```

## Модульные тесты (GoogleTest)

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

Или напрямую:

```bash
./build/tests/unit_tests
```

## Интеграционные тесты

```bash
(для macOS/Linux)

chmod +x tests/run_tests.sh
cmake --build build
./tests/run_tests.sh

(для Windows)

cmake --build build
tests\run_tests.bat

или

tests\run_tests.bat build\src\app.exe
```

Скрипт запускает 63 сценариев: единственное решение, нет решений, бесконечно
много решений, все виды ошибок входных данных.

## Документация

Если Doxygen установлен, цель `docs` сгенерирует HTML в `build/html/`:

```bash
cmake --build build --target docs
```

## Соответствие критериям

* **Универсальность.** `gaussianElimination` корректно обрабатывает
  переопределённые, недоопределённые и вырожденные системы, нулевые ведущие
  элементы (частичный выбор), отрицательные и дробные коэффициенты.
* **Надёжность.** `readMatrix` собирает все ошибки во вектор и сообщает их
  пользователю с указанием строки, столбца и длины строки. Программа никогда
  не завершается аварийно на некорректных данных.
* **Структурность.** Длинные алгоритмы разбиты на короткие функции
  (`findPivotRow`, `normalizeRow`, `eliminateColumn`, `classifySolution`,
  `extractSolution`, `processToken`, `processLine`, `validateDimensions`).
  Внутри циклов нет `break`/`continue`/`return`. Все переменные локальные.
* **Читабельность.** Имена функций и переменных полные и осмысленные.
  Комментарии в стиле Doxygen, единый стиль форматирования.
* **Покрытие тестами.** Модульные тесты GoogleTest для функции gaussianElimination,
интеграционные батарейные тесты через `run_tests.sh` для всей программы.
