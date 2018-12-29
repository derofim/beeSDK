@echo off
setlocal enabledelayedexpansion

::设置路径
call :get_pwd pwd
set build_path=%pwd%\openssl-1.1.0h
set build_tmp_output=%build_path%\output
set header_ouput_path=%pwd%\..\include
set lib_output_path=%pwd%\..\lib
set bin_output_path=%pwd%\..\bin

::如果需要编译静态库,设置为no-shared，会使用/MT，如果需要编译动态库，则置空，会使用/MD
set extra_flag=no-shared

::如果是动态库，则拷贝到lib_md目录
if "%extra_flag%"=="" (
  set lib_output_path=%pwd%\..\lib_md
)

::解压缩编译目录
if not exist %build_path% (
  echo Uncompressing...
  call zip.bat unzip -source %pwd%\openssl-1.1.0h.zip -destination %build_path%tmp -keep yes -force no
  move %build_path%tmp\openssl-1.1.0h %build_path%
  rd %build_path%tmp
)

::==================编译过程===================::
::创建临时输出目录
echo Building...
cd %build_path%
if exist %build_tmp_output% (
  rd %build_tmp_output%
)
md %build_tmp_output%

::运行Configure：
perl Configure VC-WIN32 --prefix=%build_tmp_output% %extra_flag%

::编译
nmake

::测试
nmake test

::安装
nmake install

::拷贝输出文件
echo Copying headers... 
if not exist %header_ouput_path% (
  move %build_tmp_output%\include %header_ouput_path%
)

echo Copying libraries... 
if not exist %lib_output_path% (
  md %lib_output_path%
)
copy %build_tmp_output%\lib\* %lib_output_path%\

if exist %build_tmp_output%\bin (
  copy %build_tmp_output%\bin\* %bin_output_path%\
)

::拷贝动态库符号文件
if "%extra_flag%"=="" (
  copy %build_path%\libcrypto-1_1.pdb %bin_output_path%\
  copy %build_path%\libssl-1_1.pdb %bin_output_path%\
)

::清除
nmake clean

::清除编译目录
cd ..

rmdir /s /q %build_path%

echo Done

exit /b

::获取当前路径
:get_pwd
for /f "delims=" %%i in ('cd') do set %~1=%%i
goto :eof
