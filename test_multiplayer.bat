@echo off
echo Testing X4 Multiplayer API Endpoints
echo =====================================

echo.
echo 1. Testing X4 REST Server Connection...
curl -s http://localhost:3002/ >nul 2>&1
if %errorlevel% equ 0 (
    echo    ✓ X4 REST Server is running
) else (
    echo    ✗ X4 REST Server not available
    echo    Make sure X4 is running with the mod installed
    goto :end
)

echo.
echo 2. Testing Multiplayer Endpoints...

echo    Testing MP client status...
curl -s http://localhost:3002/mp/client/status >nul 2>&1
if %errorlevel% equ 0 (
    echo    ✓ MP client status endpoint available
) else (
    echo    ✗ MP client status endpoint failed
)

echo    Testing MP server status...
curl -s http://localhost:3002/mp/server/status >nul 2>&1
if %errorlevel% equ 0 (
    echo    ✓ MP server status endpoint available
) else (
    echo    ✗ MP server status endpoint failed
)

echo    Testing enhanced player info...
curl -s http://localhost:3002/mp/player/info >nul 2>&1
if %errorlevel% equ 0 (
    echo    ✓ Enhanced player info endpoint available
) else (
    echo    ✗ Enhanced player info endpoint failed
)

echo.
echo 3. Getting Sample Data...
echo    Player Info:
curl -s http://localhost:3002/mp/player/info 2>nul

echo.
echo    Available Endpoints:
curl -s http://localhost:3002/ 2>nul | findstr "mp/"

echo.
echo Testing Complete!
echo.
echo Next Steps:
echo - Configure multiplayer_config.json for your setup
echo - Start a dedicated server or connect to existing one
echo - Use demo_multiplayer.py for interactive testing

:end
pause