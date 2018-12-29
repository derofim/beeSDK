#!/bin/bash

boringssl_include="..\/..\/third_party\/boringssl\/src\/include"
boringssl_libs="obj\/third_party\/boringssl\/boringssl.lib\ obj\/third_party\/boringssl\/boringssl_asm.lib"
boringssl_asm_stamp="obj\/third_party\/boringssl\/boringssl_asm_action.stamp"
openssl_include="D$:\/work\/mediasdk\/third_party\/openssl\/win32\/include"
openssl_libs="D$:\/work\/mediasdk\/third_party\/openssl\/win32\/lib\/ssleay32.lib D$:\/work\/mediasdk\/third_party\/openssl\/win32\/lib\/libeay32.lib"
openssl_libdir="D$:\/work\/mediasdk\/third_party\/openssl\/win32\/lib"
openssl_ld="ssleay32.lib\ libeay32.lib"
additional_obj="obj\/webrtc\/system_wrappers\/field_trial_default\/field_trial_default.obj obj\/webrtc\/system_wrappers\/metrics_default\/metrics_default.obj"

patch() {
  local file_name=$1
  echo process $file_name
  has_boringssl_header=`grep $boringssl_include $file_name`
  if [ -n "$has_boringssl_header" ];then
    #echo Found boringssl include dir,replace it and add openssl macro.
    #Replace include
    sed -i "s/$boringssl_include/$openssl_include/g" $file_name
    
    #Append openssl macro.
    sed -i "/^defines =/s/$/ -DSSL_USE_OPENSSL -DHAVE_OPENSSL_SSL_H -DFEATURE_ENABLE_SSL/" $file_name
  fi

  #Replace boringssl libraries to openssl libraries.
  sed -i "s/$boringssl_libs/$openssl_libs/g" $file_name
  
  #Delete boringssl asm stamp
  sed -i "s/$boringssl_asm_stamp//g" $file_name

  #Delete boringssl objects.
  sed -i "s/obj\/third_party\/boringssl\/boringssl\/err_data.obj.*obj\/third_party\/boringssl\/boringssl_asm\/sha512-586.o //g" $file_name
}

add_openssl_libs() {
  local file_name=$1
  echo Add openssl lib to $file_name
  #Append openssl library path.
  has_ssl_ld=`grep openssl\/win32\/lib $file_name`
  if [ -z "$has_ssl_ld" ];then
    sed -i "/ldflags =/s/$/ \/LIBPATH:\"$openssl_libdir\"/" $file_name
  fi
  has_ssl_lib=`grep ssleay32.lib $file_name`
  if [ -z "$has_ssl_lib" ];then
    sed -i "s/libs =/& $openssl_ld/" $file_name
  fi
}

grep -r boringssl . | grep -v -e boringssl.ninja -e boringssl_asm.ninja -e boringssl.vcxproj -e boringssl_asm.vcxproj | grep ninja | awk -F : '{print $1}' | sort | uniq | while read line
do
  patch $line
done

fs=('./webrtc/examples/stun_prober.ninja' './webrtc/rtc_unittests.ninja' './webrtc/webrtc_nonparallel_tests.ninja' './webrtc/stats/rtc_stats_unittests.ninja' './webrtc/modules/modules_unittests.ninja')
for f in ${fs[@]};do
  add_openssl_libs $f 
done

file_name=./webrtc/webrtc.ninja

#Add field_trial and metrics
has_field_trial=`grep field_trial_default $file_name`
if [ -z "$has_field_trial" ];then
  echo Add field_trial and metrics
  sed -i "s/alink/& $additional_obj/" $file_name
fi

#Delete external capture
echo Delete external capture
sed -i "s/obj\/webrtc\/modules\/video_capture\/video_capture\/device_info_external.obj obj\/webrtc\/modules\/video_capture\/video_capture\/video_capture_external.obj //g" $file_name