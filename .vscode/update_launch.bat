@echo off
setlocal enabledelayedexpansion

set "APP_NAME=Passive"
set "CHANGELOG_FILE=Changelog.txt"
set "LAUNCH_FILE=.vscode\launch.json"

if not exist "%CHANGELOG_FILE%" (
    echo Файл %CHANGELOG_FILE% не найден
    exit /b 1
)

set "version_line="
for /f "usebackq delims=" %%A in (`type "%CHANGELOG_FILE%" ^| findstr /c:"%APP_NAME% v"`) do (
    set "version_line=%%A"
    goto :found
)

echo Не удалось извлечь версию из %CHANGELOG_FILE%
exit /b 1

:found
for /f "tokens=1,2 delims= " %%B in (`
    powershell -NoProfile -Command ^
      "$line = '%version_line%'; ^
       if ($line -match '%APP_NAME% v(?<ver>[0-9]+(\.[0-9]+){1,4}) \((?<date>[0-9]{2}\.[0-9]{2}\.[0-9]{4})\)') { ^
         Write-Output $matches['ver'] + ' ' + $matches['date'] ^
       }"
`) do (
    set "VERSION=%%B"
    set "DATE=%%C"
)

if not defined VERSION (
    echo Не удалось распознать версию в строке:
    echo   %version_line%
    exit /b 1
)

if not exist "%LAUNCH_FILE%" (
    echo Файл %LAUNCH_FILE% не найден
    exit /b 1
)

powershell -NoProfile -Command ^
  "(Get-Content '%LAUNCH_FILE%' -Raw) `
   -replace '%APP_NAME%_\d+(\.\d+){1,4}_\d{2}\.\d{2}\.\d{4}', '%APP_NAME%_%VERSION%_%DATE%' `
   | Set-Content '%LAUNCH_FILE%'"

if %ERRORLEVEL% EQU 0 (
    echo launch.json обновлён до %APP_NAME%_%VERSION%_%DATE%
    exit /b 0
) else (
    echo Ошибка при обновлении launch.json
    exit /b 1
)

