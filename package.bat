@echo off
rem Double-click: makes both release files in release\.
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File tools\package.ps1 || goto :fail
echo.
echo Done. The files are in the release folder.
pause
exit /b 0
:fail
echo.
echo Something failed, see above.
pause
exit /b 1
