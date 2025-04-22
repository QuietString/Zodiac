@echo off
setlocal

REM ----------------------------------------------------------------------
set "ScriptDir=%~dp0"                               REM e.g. ...\Zodiac\
set "ProjectPath=%ScriptDir%Zodiac.uproject"
set "PackageOutputDir=%ScriptDir%Package"
set "LogPath=%ScriptDir%Build\PackagingLog.txt"

REM  Unreal Engine path – absolute
set "UERoot=C:\UnrealEngine_Source_5.5"
set "RunUAT=%UERoot%\Engine\Build\BatchFiles\RunUAT.bat"

REM ----------------------------------------------------------------------
REM 0) Clean old package directory
if exist "%PackageOutputDir%\City" (
    echo Removing old package directory "%PackageOutputDir%\Test" ...
    rmdir /S /Q "%PackageOutputDir%\Test"
)

REM Recreate it as an empty directory
mkdir "%PackageOutputDir%\Test"

REM ----------------------------------------------------------------------
REM Package the Test Server
call "%RunUAT%" BuildCookRun ^
  -project="%ProjectPath%" ^
  -noP4 ^
  -platform=Win64 ^
  -server -serverconfig=Development ^
  -cook -allmaps -build -stage -pak -archive ^
  -archivedirectory="%PackageOutputDir%\Test" ^
  -target=ZodiacServer ^
  -utf8output ^
  -ini:Game:[/Script/EngineSettings.GameMapsSettings]:ServerDefaultMap=/Game/Map/AnimationTest.AnimationTest

REM ----------------------------------------------------------------------
echo Done packaging Test Server!

powershell -Command "[console]::beep(1000,500)"

pause