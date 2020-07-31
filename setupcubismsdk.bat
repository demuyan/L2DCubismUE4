@echo off
setlocal
set sdkfile=CubismSdkForNative-4-r.1.zip
set extractFolder=CubismSdkForNative-4-r.1
set targetBaseFolder=Source\
set cur=%CD%
cd /d %~dp0

Call :UnZipFile "%cur%\Temp\" "%cur%\%sdkfile%"

move %cur%\Temp\%extractFolder%\Core %targetBaseFolder%\ThirdParty\SDK\
del /Q /S %cur%\Temp\%extractFolder%\Framework\src\Rendering\D3D9
del /Q /S %cur%\Temp\%extractFolder%\Framework\src\Rendering\D3D11
del /Q /S %cur%\Temp\%extractFolder%\Framework\src\Rendering\OpenGL
move %cur%\Temp\%extractFolder%\Framework %targetBaseFolder%\L2DCubism\SDK\

exit /b

:UnZipFile <ExtractTo> <newzipfile>
set vbs="%temp%\_.vbs"
if exist %vbs% del /f /q %vbs%
>%vbs%  echo Set fso = CreateObject("Scripting.FileSystemObject")
>>%vbs% echo If NOT fso.FolderExists(%1) Then
>>%vbs% echo fso.CreateFolder(%1)
>>%vbs% echo End If
>>%vbs% echo set objShell = CreateObject("Shell.Application")
>>%vbs% echo set FilesInZip=objShell.NameSpace(%2).items
>>%vbs% echo objShell.NameSpace(%1).CopyHere(FilesInZip)
>>%vbs% echo Set fso = Nothing
>>%vbs% echo Set objShell = Nothing
cscript //nologo %vbs%
if exist %vbs% del /f /q %vbs%
exit /b

