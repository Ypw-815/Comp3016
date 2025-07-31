@echo off
echo Copying DLL files to output directories...

REM Copy Debug version DLL files
echo Copying Debug DLLs...
if exist "vcpkg_installed\x64-windows\debug\bin\*.dll" (
    copy "vcpkg_installed\x64-windows\debug\bin\*.dll" "build\Debug\" >nul 2>&1
    echo Debug DLLs copied successfully.
) else (
    echo Warning: Debug DLLs not found in vcpkg_installed\x64-windows\debug\bin\
)

REM Copy Release version DLL files
echo Copying Release DLLs...
if exist "vcpkg_installed\x64-windows\bin\*.dll" (
    copy "vcpkg_installed\x64-windows\bin\*.dll" "build\Release\" >nul 2>&1
    echo Release DLLs copied successfully.
) else (
    echo Warning: Release DLLs not found in vcpkg_installed\x64-windows\bin\
)

REM Copy Irrklang DLL files (if they exist)
if exist "external\irrklang\bin\*.dll" (
    echo Copying Irrklang DLLs...
    copy "external\irrklang\bin\*.dll" "build\Debug\" >nul 2>&1
    copy "external\irrklang\bin\*.dll" "build\Release\" >nul 2>&1
    echo Irrklang DLLs copied successfully.
)

echo.
echo DLL copying complete!
echo.
echo If you're using Visual Studio, make sure to:
echo 1. Set the working directory to: %~dp0build\Debug (for Debug builds)
echo 2. Or set the working directory to: %~dp0build\Release (for Release builds)
echo.
echo You can do this in Visual Studio by:
echo - Right-click on your project in Solution Explorer
echo - Select "Properties"
echo - Go to "Debugging" section
echo - Set "Working Directory" to the appropriate path
pause 