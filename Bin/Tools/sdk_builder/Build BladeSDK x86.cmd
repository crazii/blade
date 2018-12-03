@echo off
@REM set /p sdk_target=Please Input SDK target path, default - C:\Program Files\BladeSDK\:
@REM if not defined sdk_target set sdk_target="C:\Program Files\BladeSDK\"
set sdk_target="Z:\BladeSDK"

@REM make directory & copy files
mkdir "%sdk_target:"=%\bin"
mkdir "%sdk_target:"=%\bin\x86"
mkdir "%sdk_target:"=%\bin\x64"
mkdir "%sdk_target:"=%\include"
mkdir "%sdk_target:"=%\lib"
mkdir "%sdk_target:"=%\lib\x86"
mkdir "%sdk_target:"=%\lib\x64"

@REM copy includes/headers
pushd %cd%
cd ..\..\..\Source\
call :recursive_copy ".\" %sdk_target:"=%\include\
popd

@REM copy binaries
xcopy ..\..\..\Bin\Debug_Win32 /s /y /i /exclude:bin_exclude.txt "%sdk_target:"=%\bin\x86"
xcopy ..\..\..\Bin\Debug_x64 /s /y /i /exclude:bin_exclude.txt "%sdk_target:"=%\bin\x64"

@REM copy static libs
xcopy ..\..\..\Bin\Debug_Win32\Lib /s /y /i /exclude:lib_exclude.txt "%sdk_target:"=%\lib\x86"
xcopy ..\..\..\Bin\Debug_Win32\Plugins\Lib /s /y /i /exclude:lib_exclude.txt "%sdk_target:"=%\lib\x86\Plugins"
xcopy ..\..\..\Bin\Debug_x64\Lib /s /y /i /exclude:lib_exclude.txt "%sdk_target:"=%\lib\x64"
xcopy ..\..\..\Bin\Debug_x64\Plugins\Lib /s /y /i /exclude:lib_exclude.txt "%sdk_target:"=%\lib\x64\Plugins"


@echo add environment variables...
@rem echo %BLADE_SDK_DIR%
@rem echo %sdk_target:"=%\
if not "%BLADE_SDK_DIR%"=="%sdk_target:"=%\" (

	Wmic OS Get Caption|Find /i "Windows XP">nul
	if errorlevel 1 (
		REM should be version above XP
		setx BLADE_SDK_DIR "%sdk_target:"=%\\"
	) else (
		REM window XP
		reg add "HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Session Manager\Environment" /v BLADE_SDK_DIR /t REG_EXPAND_SZ /d "%sdk_target:"=%\\" /f
		)
	)

)

set bin_path="%%BLADE_SDK_DIR%%bin\x86\;%%BLADE_SDK_DIR%%bin\x86\Plugins\"
set expand_bin_path="%sdk_target:"=%\bin\x86\;%sdk_target:"=%\bin\x86\Plugins\"

@rem echo %BLADE_SDK_BIN%
@rem echo %expand_bin_path:"=%
if not "%BLADE_SDK_BIN%"=="%expand_bin_path:"=%" (

	Wmic OS Get Caption|Find /i "Windows XP">nul
	if errorlevel 1 (
		REM should be version above XP
		setx BLADE_SDK_BIN "%bin_path:"=%\"
	) else (
		REM window XP
		reg add "HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Session Manager\Environment" /f /v BLADE_SDK_BIN /t REG_EXPAND_SZ /d "%bin_path:"=%\"
		)
	)
	pause
)

set full_path="%Path%;%%BLADE_SDK_BIN%%"

REM echo %bin_path%
REM echo
REM echo %full_path%
REM pause

@REM echo "%Path%"|Find /i "%%BLADE_SDK_BIN%%" >nul   - 'Path' already fully expanded
echo "%Path%"|Find /i "%expand_bin_path:"=%" >nul
if errorlevel 1 (

	Wmic OS Get Caption|Find /i "Windows XP">nul
	if errorlevel 1 (
		REM should be version above XP
		setx Path %full_path%
	) else (
		REM window XP
		reg add "HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Session Manager\Environment" /f /v Path /t REG_EXPAND_SZ /d %full_path%
		)

	)

)

:finish
@echo Blade SDK build finished.
@pause
@exit /b 0





REM sub routine to copy directories

:recursive_copy
pushd %cd%

cd %1

if exist header (
xcopy header /s /y /i "%~2\%~1"
) else (

for /f "delims=" %%j in ('dir /ad/b') do (
cd %1
call :recursive_copy %%j "%~2%~1\"
popd
)

popd
)
@exit /b 0