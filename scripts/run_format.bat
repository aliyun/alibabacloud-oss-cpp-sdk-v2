@echo off
setlocal enabledelayedexpansion

REM Run from the project root directory as follows:
REM scripts\run_format.bat sdk tests

echo Running clang-format...

if "%~1"=="" (
    echo Usage: scripts\run_format.bat dir1 [dir2 ...]
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
                echo format %%f
                clang-format -i --style=file "%%f"
            )
            set "skip="
        )
        echo ~~~ %%d directory formatted ~~~
    )
)

echo Done. All files were formatted (if required).
