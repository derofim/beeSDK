@echo off
setlocal enabledelayedexpansion

::����·��
call :get_pwd pwd
set build_path=%pwd%\openssl-1.1.0h
set build_tmp_output=%build_path%\output
set header_ouput_path=%pwd%\..\include
set lib_output_path=%pwd%\..\lib
set bin_output_path=%pwd%\..\bin

::�����Ҫ���뾲̬��,����Ϊno-shared����ʹ��/MT�������Ҫ���붯̬�⣬���ÿգ���ʹ��/MD
set extra_flag=no-shared

::����Ƕ�̬�⣬�򿽱���lib_mdĿ¼
if "%extra_flag%"=="" (
  set lib_output_path=%pwd%\..\lib_md
)

::��ѹ������Ŀ¼
if not exist %build_path% (
  echo Uncompressing...
  call zip.bat unzip -source %pwd%\openssl-1.1.0h.zip -destination %build_path%tmp -keep yes -force no
  move %build_path%tmp\openssl-1.1.0h %build_path%
  rd %build_path%tmp
)

::==================�������===================::
::������ʱ���Ŀ¼
echo Building...
cd %build_path%
if exist %build_tmp_output% (
  rd %build_tmp_output%
)
md %build_tmp_output%

::����Configure��
perl Configure VC-WIN32 --prefix=%build_tmp_output% %extra_flag%

::����
nmake

::����
nmake test

::��װ
nmake install

::��������ļ�
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

::������̬������ļ�
if "%extra_flag%"=="" (
  copy %build_path%\libcrypto-1_1.pdb %bin_output_path%\
  copy %build_path%\libssl-1_1.pdb %bin_output_path%\
)

::���
nmake clean

::�������Ŀ¼
cd ..

rmdir /s /q %build_path%

echo Done

exit /b

::��ȡ��ǰ·��
:get_pwd
for /f "delims=" %%i in ('cd') do set %~1=%%i
goto :eof
