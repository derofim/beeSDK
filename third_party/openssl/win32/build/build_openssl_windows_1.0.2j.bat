@echo off

::����·��
call :get_pwd pwd
set build_path=%pwd%\openssl-1.0.2j
set build_tmp_output=output
set header_ouput_path=%pwd%\..\include
set lib_output_path=%pwd%\..\lib
set libeay32_lib=libeay32.lib
set ssleay32_lib=ssleay32.lib

::��ѹ������Ŀ¼
if exist %build_path% rmdir /s /q %build_path%
echo Uncompressing...
call zip.bat unzip -source %pwd%\openssl-1.0.2j.zip -destination %build_path%tmp -keep yes -force no
move %build_path%tmp\openssl-1.0.2j %build_path%
rd %build_path%tmp

::==================�������===================::
::������ʱ���Ŀ¼
echo Building...
cd %build_path%
md %build_tmp_output%

::����configure��
perl Configure VC-WIN32 --prefix=./%build_tmp_output%

::����Makefile�ļ�
call ms\do_nasm

::�޸�pdb�ļ���,lib.pdb�ĳ�openssl.pdb
perl -i.bak -pe "s/\/lib/\/openssl/g" ms/nt.mak

::MT�ĳ�MD
perl -i.bak -pe "s/\/MT/\/MD/g" ms/nt.mak

::���뾲̬��
call nmake -f ms\nt.mak

::���Ծ�̬��
call nmake -f ms\nt.mak test

::����pdb�ļ�
echo Copying pdb... 
copy tmp32\openssl.pdb %lib_output_path%\

::��װ��̬��
call nmake -f ms\nt.mak install

::����ϴξ�̬��ı��룬�Ա����±���
call nmake -f ms\nt.mak clean
::============================================::

::��������ļ�
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

::�������Ŀ¼
cd ..

rmdir /s /q %build_path%

echo Done

exit /b

::��ȡ��ǰ·��
:get_pwd
for /f "delims=" %%i in ('cd') do set %~1=%%i
goto :eof
