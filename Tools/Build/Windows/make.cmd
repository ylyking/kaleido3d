@rem ==========kaleido3d build script
@rem Copyright (c) 2014 - 2017, Tsin Studio. All rights reserved.
@rem Copyright (c) 2014 - 2017, Qin Zhou. All rights reserved.
@echo off

set SOURCE_DIR=%~dp0\..\..\..
set PLATFORM=Win64
set CONFIG=Debug
set BUILD_DIR=%~dp0\Intermediate\%PLATFORM%_%CONFIG%

if not exist %SOURCE_DIR%\Source\ThirdParty_Prebuilt goto CHECK_DEPENDENCIES
goto BUILD_BY_CMAKE

:CHECK_DEPENDENCIES
echo Checkout Dependencies From Github
goto BUILD_BY_CMAKE

:BUILD_BY_CMAKE
qmake -v
if "%ERRORLEVEL%"=="0" (set BUILD_EDITOR=ON) else (set BUILD_EDITOR=OFF)
llvm-config --libdir
if "%ERRORLEVEL%"=="0" (set BUILD_WITH_CPP_REFLECTOR=ON) else (set BUILD_WITH_CPP_REFLECTOR=OFF)
echo Now Generate Project by CMake (VS 2017)
cmake -G"Visual Studio 15 2017 %PLATFORM%" -H%SOURCE_DIR% -B%BUILD_DIR% -DCMAKE_BUILD_TYPE=%CONFIG% -DBUILD_WITH_EDITOR=%BUILD_EDITOR% -Dgtest_force_shared_crt=OFF
if "%ERRORLEVEL%"=="0" (goto BUILD_CMAKE)
RD /S /Q %BUILD_DIR%

echo Now Generate Project by CMake (VS 2015)
cmake -G"Visual Studio 14 2015 %PLATFORM%" -H%SOURCE_DIR% -B%BUILD_DIR% -DCMAKE_BUILD_TYPE=%CONFIG% -DBUILD_WITH_EDITOR=%BUILD_EDITOR%
if "%ERRORLEVEL%"=="0" (goto BUILD_CMAKE) else (goto NotSupport)

:BUILD_CMAKE
cmake --build %BUILD_DIR% --config %CONFIG%
goto End

:NotSupport
echo Visual Studio Version not supported!
RD /S /Q %BUILD_DIR%

:End