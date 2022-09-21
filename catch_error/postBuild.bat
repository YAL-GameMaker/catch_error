@echo off
set dllPath=%~1
set solutionDir=%~2
set projectDir=%~3
set arch=%~4
set config=%~5

echo Running post-build for %config%

set extName=catch_error
set dllName=catch_error
set gmlDir14=%solutionDir%catch_error.gmx
set gmlDir22=%solutionDir%catch_error_yy
set ext14=%gmlDir14%\extensions\%extName%
set ext22=%gmlDir22%\extensions\%extName%
set dllRel=%dllName%.dll
set cppRel=%dllName%.cpp
set cppPath=%ext22%\%cppRel%
set gmlPath=%ext22%\*.gml
set docName=%extName%.html
set docPath=%solutionDir%export\%docName%

echo Copying documentation...
copy /Y %docPath% %gmlDir22%\datafiles\%docName%
copy /Y %docPath% %gmlDir14%\datafiles\%docName%

echo Combining the source files...
type "%projectDir%*.h" "%projectDir%*.cpp" >"%cppPath%" 2>nul
	
echo Running GmxGen...

gmxgen "%ext22%\%extName%.yy" ^
--copy "%dllPath%" "%dllRel%:%arch%"
	
gmxgen "%ext14%.extension.gmx" ^
--copy "%dllPath%" "%dllRel%:%arch%" ^
--copy "%cppPath%" "%cppRel%" ^
--copy "%gmlPath%" "*.gml"