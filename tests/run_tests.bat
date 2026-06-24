@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

REM ===================================================
REM Path to executable
REM ===================================================

set "SCRIPT_DIR=%~dp0"
if "%~1"=="" (
    set "APP=%SCRIPT_DIR%app.exe"
) else (
    set "APP=%~1"
)

if not exist "%APP%" (
    echo ERROR: executable not found: %APP%
    exit /b 1
)

REM ===================================================
REM Temporary directory
REM ===================================================

set "TMP=%TEMP%\gauss_tests_%RANDOM%"
mkdir "%TMP%"

REM ===================================================
REM Counters
REM ===================================================

set /a PASS=0
set /a FAIL=0
set /a TOTAL=0

goto :main

REM ===================================================
REM run_test
REM ===================================================

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
            echo PASS [%T_ID%] %T_DESC%
            set /a PASS+=1
            goto :eof
        )
    )

    echo FAIL [%T_ID%] %T_DESC%

    if not "%T_ACTUAL_EXIT%"=="%T_EXP_EXIT%" (
        echo   exit expected=%T_EXP_EXIT% actual=%T_ACTUAL_EXIT%
    )

    if not "!T_ACTUAL_OUT!"=="%T_EXP_OUT%" (
        echo   output expected='!T_EXP_OUT!' actual='!T_ACTUAL_OUT!'
    )

    set /a FAIL+=1
    goto :eof

REM ===================================================
REM run_test_no_output
REM ===================================================

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
        echo PASS [%T_ID%] %T_DESC%
        set /a PASS+=1
    ) else (
        echo FAIL [%T_ID%] %T_DESC%
        echo   exit expected=1 actual=%T_ACTUAL_EXIT%
        set /a FAIL+=1
    )

    goto :eof

REM ===================================================
REM write_ps
REM ===================================================

:write_ps
    set "WP_ID=%~1"
    set "WP_CONTENT=%~2"

    powershell -NoProfile -Command ^
        "[System.IO.File]::WriteAllText('%TMP%\in_%WP_ID%.txt', \"%WP_CONTENT%\".Replace('\n',\"`n\"))"

    goto :eof

REM ===================================================
REM Main
REM ===================================================

:main

echo.
echo ===================================================
echo Block 1: Unique solutions
echo ===================================================

call :write_ps 1 "1 1 4\n1 2 5"
call :run_test 1 "Simple 2x2 system" 0 "3 1"

call :write_ps 2 "0.5 1.5 3.0\n1.0 2.5 5.0"
call :run_test 2 "Fraction coefficients" 0 "0 2"

call :write_ps 3 "0 1 2\n1 1 3"
call :run_test 3 "Row swap required" 0 "1 2"

call :write_ps 4 "1 0 0 1\n0 1 0 2\n0 0 1 3"
call :run_test 4 "Identity matrix 3x3" 0 "1 2 3"

call :write_ps 5 "2 0 0 4\n0 3 0 9\n0 0 5 10"
call :run_test 5 "Diagonal matrix 3x3" 0 "2 3 2"

call :write_ps 6 "1 1 1 6\n0 1 1 5\n0 0 1 3"
call :run_test 6 "Upper triangular" 0 "1 2 3"

call :write_ps 7 "1 0 0 3\n1 1 0 5\n1 1 1 6"
call :run_test 7 "Lower triangular" 0 "3 2 1"

call :write_ps 8 "1 2 0\n3 4 0"
call :run_test 8 "All zeros solution" 0 "0 0"

call :write_ps 9 "2 0 3\n0 3 3"
call :run_test 9 "Remove trailing zeros" 0 "1.5 1"

call :write_ps 10 "3 0 1\n0 3 2"
call :run_test 10 "Three decimal places" 0 "0.333 0.667"

call :write_ps 11 "1 1 0\n1 -1 0"
call :run_test 11 "Negative zero check" 0 "0 0"

call :write_ps 12 "100 200 -500\n200 100 500"
call :run_test 12 "Large coefficients" 0 "5 -5"

echo.
echo ===================================================
echo Block 2: No solutions
echo ===================================================

call :write_ps 13 "1 1 4\n1 1 5"
call :run_test 13 "Direct contradiction" 0 "no solutions"

call :write_ps 14 "1 2 3\n2 4 7"
call :run_test 14 "Contradiction after elimination" 0 "no solutions"

call :write_ps 15 "1 2 3 14\n0 1 1 5\n0 0 0 1"
call :run_test 15 "0 equals 1 row" 0 "no solutions"

echo.
echo ===================================================
echo Block 3: Infinite solutions
echo ===================================================

call :write_ps 16 "1 2 3 4\n5 6 7 8"
call :run_test 16 "Underdetermined system" 0 "infinitely many solutions"

call :write_ps 17 "1 1 2\n0 0 0"
call :run_test 17 "Zero row" 0 "infinitely many solutions"

call :write_ps 18 "0 0 0\n0 0 0"
call :run_test 18 "Zero matrix" 0 "infinitely many solutions"

echo.
echo ===================================================
echo Block 4: Invalid input
echo ===================================================

call :write_ps 19 ""
call :run_test_no_output 19 "Empty file"

call :write_ps 20 "1 2 3"
call :run_test_no_output 20 "Single equation"

call :write_ps 21 "1 abc 3\n4 5 6"
call :run_test_no_output 21 "Non numeric token"

call :write_ps 22 "1 2 3\n4 5"
call :run_test_no_output 22 "Different column counts"

call :write_ps 23 "1.1234 2 3\n4 5 6"
call :run_test_no_output 23 "Too many decimal digits"

call :write_ps 24 "1234567890123456 2 3\n4 5 6"
call :run_test_no_output 24 "Too many integer digits"

call :write_ps 25 "1.2.3 4 5\n6 7 8"
call :run_test_no_output 25 "Two dots in number"

call :write_ps 26 "1e2 2 3\n4 5 6"
call :run_test_no_output 26 "Scientific notation unsupported"

echo.
echo ===================================================
echo Summary
echo ===================================================
echo Total: %TOTAL% ^| Passed: %PASS% ^| Failed: %FAIL%
echo.

rmdir /s /q "%TMP%"

if %FAIL% gtr 0 exit /b 1
exit /b 0