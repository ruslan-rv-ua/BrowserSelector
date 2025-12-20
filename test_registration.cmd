@echo off
echo ====================================
echo Browser Selector Registration Test
echo ====================================
echo.
echo This script helps test the registration functionality
echo.
echo Test Steps:
echo 1. Run BrowserSelector.exe
echo 2. Click "Set as Default" button
echo 3. Follow the prompts
echo 4. Close the app
echo 5. Run this script again to check status
echo.
echo ====================================
echo Checking Registry Status...
echo ====================================
echo.

:: Check if BrowserSelector is registered
reg query "HKCU\Software\Classes\BrowserSelector" >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] BrowserSelector is REGISTERED in the system
) else (
    echo [!!] BrowserSelector is NOT registered
)

echo.
echo Checking Default Browser Status...
echo.

:: Check http default
reg query "HKCU\Software\Microsoft\Windows\Shell\Associations\UrlAssociations\http\UserChoice" /v ProgId 2>nul | find "BrowserSelector" >nul
if %errorlevel% equ 0 (
    echo [OK] BrowserSelector is set as default for HTTP
) else (
    echo [!!] BrowserSelector is NOT set as default for HTTP
)

:: Check https default
reg query "HKCU\Software\Microsoft\Windows\Shell\Associations\UrlAssociations\https\UserChoice" /v ProgId 2>nul | find "BrowserSelector" >nul
if %errorlevel% equ 0 (
    echo [OK] BrowserSelector is set as default for HTTPS
) else (
    echo [!!] BrowserSelector is NOT set as default for HTTPS
)

echo.
echo ====================================
echo Test Opening a Link
echo ====================================
echo.
echo Press any key to test opening a link with your default browser...
pause >nul

start https://www.google.com

echo.
echo Did Browser Selector open? (Y/N)
set /p answer="> "

if /i "%answer%"=="Y" (
    echo [SUCCESS] BrowserSelector is working as default browser!
) else (
    echo [INFO] If another browser opened, BrowserSelector is not set as default
    echo       Please run the app and click "Set as Default" again
)

echo.
pause
