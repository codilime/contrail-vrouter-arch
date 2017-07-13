@echo off
if "%1"=="" goto fail
copy %1\third_party\libxml2-2.9.4\win32\bin.msvc\libxml2.dll    %1\vrouter\x64\Debug\
goto success
:fail
echo Missing argument
:success
