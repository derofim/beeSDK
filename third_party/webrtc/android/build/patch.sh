#!/bin/bash

boringssl_include=../../third_party/boringssl/src/include
boringssl_lib=obj/third_party/boringssl/libboringssl.a
boringssl_stamp=obj/third_party/boringssl/boringssl.stamp
boringssl_asm_stamp=obj/third_party/boringssl/boringssl_asm.stamp
openssl_include=/home/hezhen/mediasdk/third_party/openssl/android/include
openssl_libs='/home/hezhen/mediasdk/third_party/openssl/android/lib/armeabi-v7a/libssl.a /home/hezhen/mediasdk/third_party/openssl/android/lib/armeabi-v7a/libcrypto.a'
openssl_libdir=/home/hezhen/mediasdk/third_party/openssl/android/lib/armeabi-v7a
openssl_ld="-lssl -lcrypto"

patch() {
  local file_name=$1
  echo process $file_name
  has_boringssl_header=`grep $boringssl_include $file_name`
  if [ -n "$has_boringssl_header" ];then
    #echo Found boringssl include dir,replace it and add openssl macro.
    #Replace include
    sed -i 's#'$boringssl_include'#'$openssl_include'#g' $file_name
    #Append openssl macro.
    sed -i '/^defines =/s/$/ -DSSL_USE_OPENSSL -DHAVE_OPENSSL_SSL_H -DFEATURE_ENABLE_SSL/' $file_name
  fi

  #Replace boringssl libraries to openssl libraries.
  sed -i 's#'$boringssl_lib'#/home/hezhen/mediasdk/third_party/openssl/android/lib/armeabi-v7a/libssl.a /home/hezhen/mediasdk/third_party/openssl/android/lib/armeabi-v7a/libcrypto.a#g' $file_name

  #Delete boringssl stamp
  sed -i 's#'$boringssl_stamp'##g' $file_name
  
  #Delete boringssl asm stamp
  sed -i 's#'$boringssl_asm_stamp'##g' $file_name

  #Delete boringssl objects.
  sed -i 's#obj/third_party/boringssl/boringssl/err_data.o.*obj/third_party/boringssl/boringssl_asm/sha512-armv8.o ##g' $file_name
  sed -i 's#obj/third_party/boringssl/boringssl/err_data.o.*obj/third_party/boringssl/boringssl_asm/poly1305_arm_asm.o ##g' $file_name

  #Delete extra boringssl asm objects.
  sed -i 's#obj/third_party/boringssl/boringssl_asm/chacha-armv8.o.*obj/third_party/boringssl/boringssl_asm/sha512-armv8.o ##g' $file_name
}

add_openssl_libs() {
  local file_name=$1
  echo Add openssl lib to $file_name
  #Append openssl library path.
  has_ssl_ld=`grep openssl\/android\/lib\/armeabi-v7a $file_name`
  if [ -z "$has_ssl_ld" ];then
    sed -i '/ldflags =/s/$/ -L\/home\/hezhen\/mediasdk\/third_party\/openssl\/android\/lib\/armeabi-v7a/' $file_name
  fi
  has_ssl_lib=`grep lssl $file_name`
  if [ -z "$has_ssl_lib" ];then
    sed -i 's/libs =/& -lssl -lcrypto /' $file_name
  fi
}

grep -r boringssl . | grep -v -e boringssl.ninja -e boringssl_asm.ninja -e boringssl_crypto_tests_arch_executable.ninja -e boringssl_crypto_tests_arch_executable_sources.ninja -e boringssl_ssl_tests_arch_executable.ninja -e boringssl_ssl_tests_arch_executable_sources.ninja | grep ninja | awk -F : '{print $1}' | sort | uniq | while read line
do
  patch $line
done

fs=('./webrtc/examples/stun_prober.ninja' './webrtc/_webrtc_nonparallel_tests__library.ninja' './webrtc/stats/_rtc_stats_unittests__library.ninja' './webrtc/_video_engine_tests__library.ninja' './webrtc/sdk/android/libjingle_peerconnection_so.ninja' './webrtc/_webrtc_perf_tests__library.ninja' './webrtc/api/_peerconnection_unittests__library.ninja' './webrtc/_rtc_unittests__library.ninja' './webrtc/modules/audio_coding/_neteq_rtpplay__library.ninja' './webrtc/media/_rtc_media_unittests__library.ninja' './webrtc/modules/_modules_tests__library.ninja' './webrtc/modules/_modules_unittests__library.ninja' './webrtc/modules/audio_coding/_audio_decoder_unittests__library.ninja' './webrtc/pc/_rtc_pc_unittests__library.ninja' './webrtc/test/_test_support_unittests__library.ninja' './webrtc/voice_engine/_voice_engine_unittests__library.ninja')
for f in ${fs[@]};do
  add_openssl_libs $f 
done

