@echo off
set /p ver="Version?: "
echo Uploading %ver%...
set user=yellowafterlife
set ext=gamemaker-catch_error
cmd /C itchio-butler push catch_error-for-GMS1.gmez %user%/%ext%:gms1 --userversion=%ver%
cmd /C itchio-butler push catch_error-for-GMS2.yymp %user%/%ext%:gms2 --userversion=%ver%

pause