@echo off  
title ����������������V1  
echo         �ļ�֧������c,h,cpp ,   ֧��4�ֹ�����ʽ  
echo 1˫��ִ��ת����ǰĿ¼����Ŀ¼���ļ�  
echo 2��ק����·���ĵ����ļ�  
echo 3��ק����·����Ŀ¼  
echo 4�Ҽ��ļ���Ŀ¼���͵�sendto  
echo.  
  
echo.  
cd /d "%~dp1"  
set filename=%~nx1  
set pats=%~dp1  
::�״�ʹ��,���޸������AsPath��·��!!!!!!!!!!!!!!!!!!!!!!!!!  
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
ECHO ����Ч�ĺ�׺,��ǰ֧�ֵĺ�׺������c,cpp,h ,Ҫ֧�������������޸Ĳ���  
pause  
exit  

:single  
echo --------------------singleFile mode----------------------  
ECHO ת���ĵ����ļ�:%filename%  
%AsPath% "%Astyle_config%" "%filename%"  
::�Ͼ��еĲ��������޸�  
  
REM ɾ�����еı����ļ�  
REM del *.pre /s /q  
pause  
exit  
:dir  
echo ---------------------dir mode-----------------------------  
for /r "%~nx1" %%f in (*.cpp;*.c;*.h) do %AsPath% "%Astyle_config%" "%%f"  
REM ɾ�����еı����ļ�  
REM for /r "%~nx1" %%a in (*.pre) do del "%%a"  
pause  
exit  
:doubleClick  
echo -------------------doubleClick mode--------------------------  
for /r . %%f in (*.cpp;*.c;*.h) do %AsPath% "%Astyle_config%" "%%f"  
REM ɾ�����еı����ļ�  
REM del *.pre /s /q  
pause  
exit  