@echo off
setlocal

REM ----------------------------------------------------------------------
REM  Base folders (relative to the .bat file itself)
set "ScriptDir=%~dp0"                               REM e.g. ...\Zodiac\
set "ProjectPath=%ScriptDir%Zodiac.uproject"
set "PackageOutputDir=%ScriptDir%Package"
set "LogPath=%ScriptDir%Build\PackagingLog.txt"

REM  Unreal Engine path – absolute
set "UERoot=C:\UnrealEngine_Source_5.5"
set "RunUAT=%UERoot%\Engine\Build\BatchFiles\RunUAT.bat"

REM ----------------------------------------------------------------------
echo Cleaning previous package ...
if exist "%PackageOutputDir%\City" rmdir /S /Q "%PackageOutputDir%\City"
mkdir "%PackageOutputDir%\City"

REM ----------------------------------------------------------------------
echo Packaging CLIENT ...
call "%RunUAT%" BuildCookRun ^
 -project="%ProjectPath%" ^
 -noP4 ^
 -platform=Win64 ^
 -clientconfig=Development ^
 -cook -allmaps -build -stage -pak ^
 -archive -archivedirectory="%PackageOutputDir%\City" ^
 -target=ZodiacClient ^
 -log="%LogPath%" ^
 -utf8output

REM ----------------------------------------------------------------------
echo Packaging SERVER ...
call "%RunUAT%" BuildCookRun ^
 -project="%ProjectPath%" ^
 -noP4 ^
 -platform=Win64 ^
 -server -serverconfig=Development ^
 -cook -allmaps -build -stage -pak ^
 -archive -archivedirectory="%PackageOutputDir%\City" ^
 -target=ZodiacServer ^
 -utf8output

REM ----------------------------------------------------------------------
echo Done packaging both Client and Server!
powershell -Command "[console]::beep(1000,500)"
pause