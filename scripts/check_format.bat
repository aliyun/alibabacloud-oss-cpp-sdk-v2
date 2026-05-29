@echo off
setlocal enabledelayedexpansion

REM Run from the project root directory as follows:
REM scripts\check_format.bat sdk tests

echo Running clang-format...

set RET_CODE=0

if "%~1"=="" (
    echo Usage: scripts\check_format.bat dir1 [dir2 ...]
    exit /b 1
)

for %%d in (%*) do (
    if not exist "%%d\" (
        echo %%d is not a directory
    ) else (
        for /r "%%d" %%f in (*.h *.hpp *.c *.cpp) do (
            set "skip="
            echo %%f | findstr /i /c:"sdk\src\thirdparty\" >nul 2>&1 && set "skip=1"
            echo %%f | findstr /i /c:"tests\external\" >nul 2>&1 && set "skip=1"
            if not defined skip (
                clang-format -i --dry-run --Werror --style=file "%%f"
                if errorlevel 1 set RET_CODE=1
            )
            set "skip="
        )
        echo ~~~ %%d directory checked ~~~
    )
)

if %RET_CODE% equ 0 (
    echo Everything up to standard
) else (
    echo Not up to formatting standard
    echo Try running run_format.bat to format all files.
)

exit /b %RET_CODE%
