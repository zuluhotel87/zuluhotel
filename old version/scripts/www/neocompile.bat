doskey
set POLROOT=c:\zhfa1.2
set PATH=%PATH%;%POLROOT%\scripts
set ECOMPILE_PATH_EM=%POLROOT%\scripts
set ECOMPILE_PATH_INC=%POLROOT%\scripts

echo POL development environment set to %POLROOT%

ecompile -b -r d:/zh095/pkg/std/nature -r -x
