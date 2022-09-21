@echo off

if not exist "catch_error-for-GMS1" mkdir "catch_error-for-GMS1"
cmd /C copyre ..\catch_error.gmx\extensions\catch_error.extension.gmx catch_error-for-GMS1\catch_error.extension.gmx
cmd /C copyre ..\catch_error.gmx\extensions\catch_error catch_error-for-GMS1\catch_error
cmd /C copyre ..\catch_error.gmx\datafiles\catch_error.html catch_error-for-GMS1\catch_error\Assets\datafiles\catch_error.html
cd catch_error-for-GMS1
cmd /C 7z a catch_error-for-GMS1.7z *
move /Y catch_error-for-GMS1.7z ../catch_error-for-GMS1.gmez
cd ..

if not exist "catch_error-for-GMS2\extensions" mkdir "catch_error-for-GMS2\extensions"
if not exist "catch_error-for-GMS2\datafiles" mkdir "catch_error-for-GMS2\datafiles"
if not exist "catch_error-for-GMS2\datafiles_yy" mkdir "catch_error-for-GMS2\datafiles_yy"
cmd /C copyre ..\catch_error_yy\extensions\catch_error catch_error-for-GMS2\extensions\catch_error
cmd /C copyre ..\catch_error_yy\datafiles\catch_error.html catch_error-for-GMS2\datafiles\catch_error.html
cmd /C copyre ..\catch_error_yy\datafiles_yy\catch_error.html.yy catch_error-for-GMS2\datafiles_yy\catch_error.html.yy
cd catch_error-for-GMS2
cmd /C 7z a catch_error-for-GMS2.zip *
move /Y catch_error-for-GMS2.zip ../catch_error-for-GMS2.yymp
cd ..

pause