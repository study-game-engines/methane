@REM To build execute 'Build.bat' from 'Visual Studio Native Tools Command Prompt'
@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

SET PLATFORM_TYPE=Win64
SET ARCH_TYPE=x64
SET BUILD_TYPE=Release
SET BUILD_VERSION=0.4
SET CMAKE_FLAGS= ^
    -DMETHANE_SHADERS_CODEVIEW_ENABLED:BOOL=ON ^
    -DMETHANE_RUN_TESTS_DURING_BUILD:BOOL=OFF ^
    -DMETHANE_COMMAND_DEBUG_GROUPS_ENABLED:BOOL=ON ^
    -DMETHANE_LOGGING_ENABLED:BOOL=OFF ^
    -DMETHANE_USE_OPEN_IMAGE_IO:BOOL=OFF ^
    -DMETHANE_SCOPE_TIMERS_ENABLED:BOOL=OFF ^
    -DMETHANE_ITT_INSTRUMENTATION_ENABLED:BOOL=ON ^
    -DMETHANE_ITT_METADATA_ENABLED:BOOL=OFF ^
    -DMETHANE_GPU_INSTRUMENTATION_ENABLED:BOOL=OFF ^
    -DMETHANE_TRACY_PROFILING_ENABLED:BOOL=OFF ^
    -DMETHANE_TRACY_PROFILING_ON_DEMAND:BOOL=OFF

SET CONFIG_DIR=%~dp0..\Output\VisualStudio\%PLATFORM_TYPE%-%BUILD_TYPE%
SET INSTALL_DIR=%CONFIG_DIR%\Install
SET SOURCE_DIR=%~dp0..\..
SET START_DIR=%cd%

SET GRAPHVIZ_DIR=%CONFIG_DIR%\GraphViz
SET GRAPHVIZ_DOT_DIR=%GRAPHVIZ_DIR%\dot
SET GRAPHVIZ_IMG_DIR=%GRAPHVIZ_DIR%\img
SET GRAPHVIZ_FILE=MethaneKit.dot
SET GRAPHVIZ_DOT_EXE=dot.exe

IF "%~1"=="--vs2019" SET USE_VS2019=1
IF "%~2"=="--vs2019" SET USE_VS2019=1

IF DEFINED USE_VS2019 (
    SET CMAKE_GENERATOR=Visual Studio 16 2019
    SET CMAKE_FLAGS=-A %ARCH_TYPE% %CMAKE_FLAGS%
) ELSE (
    SET CMAKE_GENERATOR=Visual Studio 15 2017 %PLATFORM_TYPE%
)

IF "%~1"=="--analyze" (

    SET IS_ANALYZE_BUILD=1
    SET BUILD_DIR=%CONFIG_DIR%\Analyze
    SET SONAR_SCANNER_DIR=%SOURCE_DIR%\Externals\SonarScanner\binaries\Windows
    SET SONAR_SCANNER_ZIP=!SONAR_SCANNER_DIR!.zip
    SET SONAR_BUILD_WRAPPER_EXE=!SONAR_SCANNER_DIR!\build-wrapper-win-x86-64.exe
    SET SONAR_SCANNER_MSBUILD_EXE=!SONAR_SCANNER_DIR!\SonarScanner.MSBuild.exe

    ECHO =========================================================
    ECHO Code analysis for build Methane %PLATFORM_TYPE% %BUILD_TYPE%
    ECHO =========================================================
    ECHO  * Build in: '!BUILD_DIR!'
    ECHO =========================================================

) ELSE (

    SET IS_ANALYZE_BUILD=0
    SET BUILD_DIR=%CONFIG_DIR%\Build

    ECHO =========================================================
    ECHO Clean build and install Methane %PLATFORM_TYPE% %BUILD_TYPE%
    ECHO =========================================================
    ECHO  * Build in:   '!BUILD_DIR!'
    ECHO  * Install to: '%INSTALL_DIR%'
    ECHO =========================================================
)

RD /S /Q "%CONFIG_DIR%"
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

MKDIR "%BUILD_DIR%"
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

IF %IS_ANALYZE_BUILD% EQU 1 (

    ECHO Unpacking Sonar Scanner binaries...
    IF EXIST "%SONAR_SCANNER_DIR%" RD /S /Q "%SONAR_SCANNER_DIR%"
    CALL powershell -Command "Expand-Archive %SONAR_SCANNER_ZIP% -DestinationPath %SONAR_SCANNER_DIR%"
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR

    SET GITBRANCH=
    FOR /F "tokens=* USEBACKQ" %%F IN (`git rev-parse --abbrev-ref HEAD`) DO (
        IF NOT "%%F" == "" SET GITBRANCH=%%F
    )
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR

    ECHO ----------
    ECHO Analyzing code with Sonar Scanner on branch !GITBRANCH!...

    IF NOT EXIST "%BUILD_DIR%" MKDIR "%BUILD_DIR%"
    CD "%BUILD_DIR%"

    ECHO ----------
    ECHO Generating build files for %CMAKE_GENERATOR% with SonarScanner wrapper...
    "%SONAR_BUILD_WRAPPER_EXE%" --out-dir "%BUILD_DIR%"^
         cmake -G "%CMAKE_GENERATOR%" %CMAKE_FLAGS% "%SOURCE_DIR%"
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR

    ECHO ----------
    ECHO Building with %CMAKE_GENERATOR% and SonarScanner wrapper...
    CD "%SOURCE_DIR%"
    "%SONAR_SCANNER_MSBUILD_EXE%" begin^
        /k:egorodet_MethaneKit^
        /o:egorodet-github^
        /v:%BUILD_VERSION%^
        /d:sonar.branch.name="!GITBRANCH!"^
        /d:sonar.sources="%SOURCE_DIR%"^
        /d:sonar.projectBaseDir="%SOURCE_DIR%"^
        /d:sonar.cfamily.build-wrapper-output="%BUILD_DIR%"^
        /d:sonar.host.url="https://sonarcloud.io"^
        /d:sonar.login=6e1dbce6af614f59d75f1d78f0609aaaa60caee1
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR

    MSBuild.exe "%BUILD_DIR%\MethaneKit.sln" /t:Rebuild
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR

    "%SONAR_SCANNER_MSBUILD_EXE%" end^
        /d:sonar.login=6e1dbce6af614f59d75f1d78f0609aaaa60caee1
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR

) ELSE (
    CD "%BUILD_DIR%"

    ECHO Generating build files for %CMAKE_GENERATOR%...
    cmake -G "%CMAKE_GENERATOR%" --graphviz="%GRAPHVIZ_DOT_DIR%\%GRAPHVIZ_FILE%" -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR% %CMAKE_FLAGS% "%SOURCE_DIR%"
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR

    ECHO ----------
    ECHO Locating GraphViz dot converter...
    where %GRAPHVIZ_DOT_EXE%
    IF %ERRORLEVEL% EQU 0 (
        ECHO Converting GraphViz diagram to image...
        MKDIR "%GRAPHVIZ_IMG_DIR%"
        IF %ERRORLEVEL% NEQ 0 GOTO ERROR
        FOR %%f in ("%GRAPHVIZ_DOT_DIR%\*.*") do (
            ECHO Writing image "%GRAPHVIZ_IMG_DIR%\%%~nxf.png"
            "%GRAPHVIZ_DOT_EXE%" -Tpng "%%f" -o "%GRAPHVIZ_IMG_DIR%\%%~nxf.png"
            IF %ERRORLEVEL% NEQ 0 GOTO ERROR
        )
    ) ELSE (
        ECHO "GraphViz `dot` executable was not found. Skipping graph images generation."
    )

    ECHO ----------
    ECHO Building with %CMAKE_GENERATOR%...
    cmake --build . --config %BUILD_TYPE% --target install
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR

    ECHO ----------
    ECHO Running tests...
    ctest --build-config %BUILD_TYPE% --output-on-failure
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR
)

GOTO STOP

:ERROR
ECHO Error occurred %ERRORLEVEL%. Script execution was stopped.
GOTO STOP

:STOP
CD "%START_DIR%"
ENDLOCAL
ECHO ON