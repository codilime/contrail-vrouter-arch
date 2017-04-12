@echo off
if "%1"=="" goto fail
copy %1\third_party\libxml2-2.9.3-win32-x86_64\bin\libxml2-2.dll    %1\vrouter\x64\Debug\
copy %1\third_party\zlib-1.2.8-win32-x86_64\bin\zlib1.dll           %1\vrouter\x64\Debug\
copy %1\third_party\iconv-1.14-win32-x86_64\bin\libiconv-2.dll      %1\vrouter\x64\Debug\
goto success
:fail
echo Missing argument
:success
