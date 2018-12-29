@echo off

::设置路径
call :get_pwd pwd
set build_path=%pwd%\openssl-1.0.2j
set build_tmp_output=output
set header_ouput_path=%pwd%\..\include
set lib_output_path=%pwd%\..\lib
set libeay32_lib=libeay32.lib
set ssleay32_lib=ssleay32.lib

::解压缩编译目录
if exist %build_path% rmdir /s /q %build_path%
echo Uncompressing...
call zip.bat unzip -source %pwd%\openssl-1.0.2j.zip -destination %build_path%tmp -keep yes -force no
move %build_path%tmp\openssl-1.0.2j %build_path%
rd %build_path%tmp

::==================编译过程===================::
::创建临时输出目录
echo Building...
cd %build_path%
md %build_tmp_output%

::运行configure：
perl Configure VC-WIN32 --prefix=./%build_tmp_output%

::创建Makefile文件
call ms\do_nasm

::修改pdb文件名,lib.pdb改成openssl.pdb
perl -i.bak -pe "s/\/lib/\/openssl/g" ms/nt.mak

::MT改成MD
perl -i.bak -pe "s/\/MT/\/MD/g" ms/nt.mak

::编译静态库
call nmake -f ms\nt.mak

::测试静态库
call nmake -f ms\nt.mak test

::拷贝pdb文件
echo Copying pdb... 
copy tmp32\openssl.pdb %lib_output_path%\

::安装静态库
call nmake -f ms\nt.mak install

::清除上次静态库的编译，以便重新编译
call nmake -f ms\nt.mak clean
::============================================::

::拷贝输出文件
echo Copying headers... 
if not exist %header_ouput_path% (
  move %build_tmp_output%\include %header_ouput_path%
)

echo Copying libraries... 
if not exist %lib_output_path% (
  md %lib_output_path%
)
copy %build_tmp_output%\lib\%libeay32_lib% %lib_output_path%\%libeay32_lib%
copy %build_tmp_output%\lib\%ssleay32_lib% %lib_output_path%\%ssleay32_lib%

::清除编译目录
cd ..

rmdir /s /q %build_path%

echo Done

exit /b

::获取当前路径
:get_pwd
for /f "delims=" %%i in ('cd') do set %~1=%%i
goto :eof
