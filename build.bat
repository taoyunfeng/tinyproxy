@echo off
setlocal enabledelayedexpansion

if defined VS80COMNTOOLS (
set vcvars="%VS80COMNTOOLS%vsvars32.bat"
set vcdir="%VS80COMNTOOLS%"
if exist !vcvars! goto find
)

if defined VS90COMNTOOLS (
set vcvars="%VS90COMNTOOLS%vsvars32.bat"
set vcdir="%VS90COMNTOOLS%"
if exist !vcvars! goto find
)

if defined VS100COMNTOOLS (
set vcvars="%VS100COMNTOOLS%vsvars32.bat"
set vcdir="%VS100COMNTOOLS%"
if exist !vcvars! goto find
)

if defined VS110COMNTOOLS (
set vcvars="%VS110COMNTOOLS%vsvars32.bat"
set vcdir="%VS110COMNTOOLS%"
if exist !vcvars! goto find
)

if defined VS120COMNTOOLS (
set vcvars="%VS120COMNTOOLS%vsvars32.bat"
set vcdir="%VS120COMNTOOLS%"
if exist !vcvars! goto find
)

if defined VS140COMNTOOLS (
set vcvars="%VS140COMNTOOLS%vsvars32.bat"
set vcdir="%VS140COMNTOOLS%"
if exist !vcvars! goto find
)

echo Visual Stdio 2008~2015 has not been installed.
goto :eof

:find

set CC="%vcdir:"=%..\..\vc\bin\cl.exe"
set LNK="%vcdir:"=%..\..\vc\bin\link.exe"
set BINDIR=.\bin
set OBJDIR=.\obj
set SRCDIR=.\src
set TAG=proxy.exe
set CFLAG=
set LFLAG=/DYNAMICBASE 
set DEBUG_FLAG=0

for %%i in (%*) do (
	if "%%i" == "DEBUG" (
		set DEBUG_FLAG=1
	)
)

if %DEBUG_FLAG% EQU 1 (
	set CFLAG=/D"_DEBUG" /Od /Zi /MTd
	set LFLAG=%LFLAG% /DEBUG 
) else (
	set CFLAG=/D"NDEBUG" /O2 /MT
)

if NOT exist "%OBJDIR%" (
	mkdir "%OBJDIR%"
) else (
	del "%OBJDIR%\*" /Q /f
)

if NOT exist "%BINDIR%" (
	mkdir "%BINDIR%"
)

if exist "%BINDIR%\%TAG%" (
	del "%BINDIR%\%TAG%"
)

%CC% %SRCDIR%\*.c /I"." /c /D"WIN32" /D"_WIN32" /D"_WINDOWS" %CFLAG%

if %errorlevel% NEQ 0 (
	exit
)

for %%i in (*.obj) do (
	move /Y "%%i" "%OBJDIR%\"
)

for %%i in ('%CC%') do (
	echo %%i | findstr "19." > nul && (
		set LFLAG=%LFLAG% legacy_stdio_definitions.lib
		goto link
	)
)

:link

%LNK% %OBJDIR%\*.obj /MANIFEST:NO /OUT:"%BINDIR%\%TAG%" /MACHINE:x86 %LFLAG%
