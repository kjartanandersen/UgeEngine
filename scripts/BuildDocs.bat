@echo off
REM Generates the Doxygen API documentation for the Uge engine.
REM Output: docs\html\index.html   Warnings: docs\doxygen-warnings.log

pushd "%~dp0\..\docs"

where doxygen >nul 2>nul
if errorlevel 1 (
    echo [BuildDocs] ERROR: doxygen was not found on PATH.
    echo [BuildDocs] Install it from https://www.doxygen.nl/download.html
    popd
    exit /b 1
)

where dot >nul 2>nul
if errorlevel 1 (
    echo [BuildDocs] WARNING: Graphviz 'dot' was not found on PATH.
    echo [BuildDocs] Diagrams will be skipped. Install from https://graphviz.org/download/
)

echo [BuildDocs] Running doxygen...
doxygen Doxyfile
if errorlevel 1 (
    echo [BuildDocs] ERROR: doxygen failed.
    popd
    exit /b 1
)

if exist doxygen-warnings.log (
    for %%A in (doxygen-warnings.log) do (
        if %%~zA GTR 0 echo [BuildDocs] Doxygen reported warnings, see docs\doxygen-warnings.log
    )
)

echo [BuildDocs] Done. Open docs\html\index.html
popd
