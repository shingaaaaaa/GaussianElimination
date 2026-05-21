@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

rem ── Путь к исполняемому файлу ─────────────────────────────────────────────
set "SCRIPT_DIR=%~dp0"
if "%~1"=="" (
    set "APP=%SCRIPT_DIR%..\build\src\app.exe"
) else (
    set "APP=%~1"
)

if not exist "%APP%" (
    echo Ошибка: исполняемый файл не найден: %APP%
    echo Соберите проект: cmake --build build
    exit /b 1
)

rem ── Временная директория ──────────────────────────────────────────────────
set "TMP=%TEMP%\gauss_tests_%RANDOM%"
mkdir "%TMP%"

rem ── Счётчики ──────────────────────────────────────────────────────────────
set /a PASS=0
set /a FAIL=0
set /a TOTAL=0

rem ── Вспомогательная процедура: запись файла ───────────────────────────────
rem    Используем PowerShell для точной записи содержимого (без BOM, с \n)
rem    Синтаксис: call :write_file <путь> <содержимое_base64>
rem    Вместо этого используем прямую запись через PowerShell inline

goto :main

rem =============================================================================
rem  :run_test id desc expected_exit expected_out
rem  Входной файл создаётся заранее в %TMP%\in_<id>.txt перед вызовом
rem =============================================================================
:run_test
    set "T_ID=%~1"
    set "T_DESC=%~2"
    set "T_EXP_EXIT=%~3"
    set "T_EXP_OUT=%~4"
    set /a TOTAL+=1

    set "IN_FILE=%TMP%\in_%T_ID%.txt"
    set "OUT_FILE=%TMP%\out_%T_ID%.txt"
    if exist "%OUT_FILE%" del "%OUT_FILE%"

    "%APP%" "%IN_FILE%" "%OUT_FILE%" >nul 2>&1
    set "T_ACTUAL_EXIT=%ERRORLEVEL%"

    set "T_ACTUAL_OUT="
    if exist "%OUT_FILE%" (
        set /p T_ACTUAL_OUT=<"%OUT_FILE%"
    )

    if "%T_ACTUAL_EXIT%"=="%T_EXP_EXIT%" (
        if "!T_ACTUAL_OUT!"=="%T_EXP_OUT%" (
            echo   PASS  [%T_ID%] %T_DESC%
            set /a PASS+=1
            goto :eof
        )
    )

    echo   FAIL  [%T_ID%] %T_DESC%
    if not "%T_ACTUAL_EXIT%"=="%T_EXP_EXIT%" (
        echo          exit: ожидался=%T_EXP_EXIT% получен=%T_ACTUAL_EXIT%
    )
    if not "!T_ACTUAL_OUT!"=="%T_EXP_OUT%" (
        echo          вывод: ожидался='%T_EXP_OUT%' получен='!T_ACTUAL_OUT!'
    )
    set /a FAIL+=1
    goto :eof

rem =============================================================================
rem  :run_test_no_output id desc
rem  Входной файл уже записан в %TMP%\in_<id>.txt
rem =============================================================================
:run_test_no_output
    set "T_ID=%~1"
    set "T_DESC=%~2"
    set /a TOTAL+=1

    set "IN_FILE=%TMP%\in_%T_ID%.txt"
    set "OUT_FILE=%TMP%\out_%T_ID%.txt"
    if exist "%OUT_FILE%" del "%OUT_FILE%"

    "%APP%" "%IN_FILE%" "%OUT_FILE%" >nul 2>&1
    set "T_ACTUAL_EXIT=%ERRORLEVEL%"

    if "%T_ACTUAL_EXIT%"=="1" (
        echo   PASS  [%T_ID%] %T_DESC%
        set /a PASS+=1
    ) else (
        echo   FAIL  [%T_ID%] %T_DESC%
        echo          exit: ожидался=1 получен=%T_ACTUAL_EXIT%
        set /a FAIL+=1
    )
    goto :eof

rem =============================================================================
rem  :write_ps id "строка1\nстрока2\n..."
rem  Записывает многострочный файл через PowerShell
rem =============================================================================
:write_ps
    set "WP_ID=%~1"
    set "WP_CONTENT=%~2"
    powershell -NoProfile -Command ^
        "[System.IO.File]::WriteAllText('%TMP%\in_%WP_ID%.txt', \"%WP_CONTENT%\".Replace('\n',\"`n\"))"
    goto :eof

rem =============================================================================
:main
rem =============================================================================

echo.
echo ════════════════════════════════════════════════════
echo   Блок 1: Единственное решение (uniqueSolution)
echo ════════════════════════════════════════════════════

call :write_ps 1 "1 1 4\n1 2 5"
call :run_test 1 "2x2 простая система: x1=3, x2=1" 0 "3 1"

call :write_ps 2 "0.5 1.5 3.0\n1.0 2.5 5.0"
call :run_test 2 "2x2 дробные коэффициенты: x1=0, x2=2" 0 "0 2"

call :write_ps 3 "1 1 -1\n1 -1 3"
call :run_test 3 "2x2 отрицательное решение: x1=1, x2=-2" 0 "1 -2"

call :write_ps 4 "1 1 -3\n2 -1 -3"
call :run_test 4 "2x2 оба значения отрицательные: x1=-2, x2=-1" 0 "-2 -1"

call :write_ps 5 "0 1 2\n1 1 3"
call :run_test 5 "2x2 нулевой первый элемент - перестановка строк: x1=1, x2=2" 0 "1 2"

call :write_ps 6 "0.001 1 2\n10 2 14"
call :run_test 6 "2x2 частичный выбор ведущего (0.001 vs 10): x1=1, x2~1.999" 0 "1 1.999"

call :write_ps 7 "1 0 0 1\n0 1 0 2\n0 0 1 3"
call :run_test 7 "3x3 классическая система: x1=1, x2=2, x3=3" 0 "1 2 3"

call :write_ps 8 "1 2 3 1\n0 1 0 0\n0 0 1 0"
call :run_test 8 "3x3 нули в решении: x1=1, x2=0, x3=0" 0 "1 0 0"

call :write_ps 9 "2 0 0 4\n0 3 0 9\n0 0 5 10"
call :run_test 9 "3x3 диагональная матрица: x1=2, x2=3, x3=2" 0 "2 3 2"

call :write_ps 10 "0 0 1 3\n0 2 1 4\n1 1 1 6"
call :run_test 10 "3x3 несколько перестановок строк: x1=2.5, x2=0.5, x3=3" 0 "2.5 0.5 3"

call :write_ps 11 "1 1 2\n2 1 3\n3 2 5"
call :run_test 11 "Переопределённая совместная (3 уравн, 2 неизв): x1=1, x2=1" 0 "1 1"

call :write_ps 12 "-1 2 3\n3 -1 2"
call :run_test 12 "2x2 дробное решение: x1=1.4, x2=2.2" 0 "1.4 2.2"

call :write_ps 13 "-0.5 1.5 2.5\n1.5 -0.5 0.5"
call :run_test 13 "2x2 дробные отрицательные: x1=1, x2=2" 0 "1 2"

call :write_ps 14 "1 1 1 6\n0 1 1 5\n0 0 1 3"
call :run_test 14 "3x3 верхнетреугольная: x1=1, x2=2, x3=3" 0 "1 2 3"

call :write_ps 15 "1 0 0 3\n1 1 0 5\n1 1 1 6"
call :run_test 15 "3x3 нижнетреугольная: x1=3, x2=2, x3=1" 0 "3 2 1"

call :write_ps 16 "100 200 -500\n200 100 500"
call :run_test 16 "2x2 большие целые коэффициенты: x1=5, x2=-5" 0 "5 -5"

call :write_ps 17 "1 0 0 0 0 0 0 0 0 0 1\n0 1 0 0 0 0 0 0 0 0 2\n0 0 1 0 0 0 0 0 0 0 3\n0 0 0 1 0 0 0 0 0 0 4\n0 0 0 0 1 0 0 0 0 0 5\n0 0 0 0 0 1 0 0 0 0 6\n0 0 0 0 0 0 1 0 0 0 7\n0 0 0 0 0 0 0 1 0 0 8\n0 0 0 0 0 0 0 0 1 0 9\n0 0 0 0 0 0 0 0 0 1 10"
call :run_test 17 "Граничный 10x10 диагональная: x1..x10=1..10" 0 "1 2 3 4 5 6 7 8 9 10"

call :write_ps 18 "1 0 7\n0 1 3"
call :run_test 18 "2x2 единичная матрица: x1=7, x2=3" 0 "7 3"

call :write_ps 19 "1 2 0\n3 4 0"
call :run_test 19 "2x2 свободный член = 0: x1=0, x2=0" 0 "0 0"

call :write_ps 20 "0.5 1.0 0.0\n2.0 0.5 0.0"
call :run_test 20 "2x2 решение только из нулей" 0 "0 0"

call :write_ps 21 "1 1 0\n1 -1 0"
call :run_test 21 "2x2 оба неизвестных = 0 (проверка -0)" 0 "0 0"

call :write_ps 22 "1 0 2\n0 1 3"
call :run_test 22 "Целое решение без десятичной точки" 0 "2 3"

call :write_ps 23 "2 0 3\n0 2 5"
call :run_test 23 "Дробное решение: x1=1.5, x2=2.5" 0 "1.5 2.5"

call :write_ps 24 "2 0 3\n0 3 3"
call :run_test 24 "Лишние нули убираются (1.500 -> 1.5)" 0 "1.5 1"

call :write_ps 25 "3 0 1\n0 3 2"
call :run_test 25 "Дробное с тремя знаками: 0.333 и 0.667" 0 "0.333 0.667"


echo.
echo ════════════════════════════════════════════════════
echo   Блок 2: Нет решений (noSolutions)
echo ════════════════════════════════════════════════════

call :write_ps 30 "1 1 4\n1 1 5"
call :run_test 30 "Прямое противоречие: x1+x2=4 vs x1+x2=5" 0 "no solutions"

call :write_ps 31 "1 2 3\n2 4 7"
call :run_test 31 "Противоречие после исключения" 0 "no solutions"

call :write_ps 32 "1 1 2\n2 2 4\n1 1 3"
call :run_test 32 "Переопределённая несовместная (3 уравн, 2 неизв)" 0 "no solutions"

call :write_ps 33 "0 0 5\n0 0 3"
call :run_test 33 "Нулевые коэффициенты, ненулевая правая часть" 0 "no solutions"

call :write_ps 34 "1 0 0 1\n0 1 0 2\n1 0 0 5"
call :run_test 34 "3x3 два уравнения противоречат третьему" 0 "no solutions"

call :write_ps 35 "2 1 3\n4 2 9"
call :run_test 35 "2x2 параллельные прямые" 0 "no solutions"

call :write_ps 36 "1 2 3 14\n0 1 1 5\n0 0 0 1"
call :run_test 36 "3x3 последняя строка: 0=1" 0 "no solutions"

call :write_ps 37 "1 1 1 1\n1 1 1 2\n1 1 1 3"
call :run_test 37 "Три параллельные плоскости" 0 "no solutions"
