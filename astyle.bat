@echo off  
title 代码批量整理助手V1  
echo         文件支持类型c,h,cpp ,   支持4种工作方式  
echo 1双击执行转换当前目录及子目录中文件  
echo 2拖拽任意路径的单个文件  
echo 3拖拽任意路径的目录  
echo 4右键文件或目录发送到sendto  
echo.  
  
echo.  
cd /d "%~dp1"  
set filename=%~nx1  
set pats=%~dp1  
::首次使用,请修改下面的AsPath的路径!!!!!!!!!!!!!!!!!!!!!!!!!  
set AsPath="D:\Program Files\AStyle\bin\AStyle.exe"  
set Astyle_config="--style=allman -k3 -W1 -xG -S -T -xb -U -p -xf -xh -xC200 -xL -H -Y -xW -w -n"

if /i "%~1"=="" goto :doubleClick   
IF EXIST "%~1\" GOTO :dir  
if "%filename:~-4%"==".cpp" goto :single  
if "%filename:~-2%"==".c"   goto :single  
if "%filename:~-2%"==".h"   goto :single  
cls  
color 0a  
ECHO %filename%  
ECHO 是无效的后缀,当前支持的后缀类型是c,cpp,h ,要支持其他类型请修改参数  
pause  
exit  

:single  
echo --------------------singleFile mode----------------------  
ECHO 转换的单个文件:%filename%  
%AsPath% "%Astyle_config%" "%filename%"  
::上句中的参数按需修改  
  
REM 删除所有的备份文件  
REM del *.pre /s /q  
pause  
exit  
:dir  
echo ---------------------dir mode-----------------------------  
for /r "%~nx1" %%f in (*.cpp;*.c;*.h) do %AsPath% "%Astyle_config%" "%%f"  
REM 删除所有的备份文件  
REM for /r "%~nx1" %%a in (*.pre) do del "%%a"  
pause  
exit  
:doubleClick  
echo -------------------doubleClick mode--------------------------  
for /r . %%f in (*.cpp;*.c;*.h) do %AsPath% "%Astyle_config%" "%%f"  
REM 删除所有的备份文件  
REM del *.pre /s /q  
pause  
exit  