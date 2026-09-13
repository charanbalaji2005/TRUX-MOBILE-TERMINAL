@echo off
echo Installing TRUX v2.0.1 to connected device...
"%LOCALAPPDATA%\Android\Sdk\platform-tools\adb.exe" install -r -d "C:\Users\Charan Balaji\Downloads\Trux-v2.0.1.apk"
if %ERRORLEVEL% NEQ 0 (
    echo Install failed. Please ensure USB debugging is enabled on your device.
) else (
    echo Launching TRUX...
    "%LOCALAPPDATA%\Android\Sdk\platform-tools\adb.exe" shell am force-stop com.meridian.shell
    "%LOCALAPPDATA%\Android\Sdk\platform-tools\adb.exe" shell am start -n com.meridian.shell/.MainActivity
    echo Done!
)
pause
