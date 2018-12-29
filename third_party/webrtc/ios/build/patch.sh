#!/bin/bash

boringssl_include=../../third_party/boringssl/src/include
boringssl_lib=obj/third_party/boringssl/libboringssl.a
boringssl_stamp=obj/third_party/boringssl/boringssl_asm.stamp
openssl_include=/Users/testtest/hezhen/mediasdk/third_party/openssl/ios/include
openssl_libss="/Users/testtest/hezhen/mediasdk/third_party/openssl/ios/lib/libssl.a"
openssl_libs='/Users/testtest/hezhen/mediasdk/third_party/openssl/ios/lib/libssl.a /Users/testtest/hezhen/mediasdk/third_party/openssl/ios/lib/libcrypto.a'
openssl_libdir=/Users/testtest/hezhen/mediasdk/third_party/openssl/ios/lib
openssl_ld="-lssl -lcrypto"

patch() {
  local file_name=$1
  echo process $file_name
  has_boringssl_header=`grep $boringssl_include $file_name`
  if [ -n "$has_boringssl_header" ];then
    #echo Found boringssl include dir,replace it and add openssl macro.
    #Replace include
    sed -i '' -e 's#'$boringssl_include'#'$openssl_include'#g' $file_name
    #Append openssl macro.
    sed -i '' -e '/^defines =/s/$/ -DSSL_USE_OPENSSL -DHAVE_OPENSSL_SSL_H -DFEATURE_ENABLE_SSL/' $file_name
  fi

  #Replace boringssl libraries to openssl libraries.
  sed -i '' -e 's#'$boringssl_lib'#/Users/testtest/hezhen/mediasdk/third_party/openssl/ios/lib/libssl.a /Users/testtest/hezhen/mediasdk/third_party/openssl/ios/lib/libcrypto.a#g' $file_name

  #Delete boringssl asm stamp
  sed -i '' -e 's#'$boringssl_stamp'##g' $file_name

  #Delete boringssl arm64 objects.
  sed -i '' -e 's#obj/third_party/boringssl/boringssl/err_data.o.*obj/third_party/boringssl/boringssl_asm/sha512-armv8.o ##g' $file_name
  
  #Delete extra boringssl asm64 objects.
  sed -i '' -e 's#obj/third_party/boringssl/boringssl_asm/chacha-armv8.o.*obj/third_party/boringssl/boringssl_asm/sha512-armv8.o ##g' $file_name
  
  #Delete boringssl armv7 objects.
  sed -i '' -e 's#obj/third_party/boringssl/boringssl/err_data.o.*obj/third_party/boringssl/boringssl_asm/sha512-armv4.o ##g' $file_name

  #Delete extra boringssl asmv7 objects.
  sed -i '' -e 's#obj/third_party/boringssl/boringssl_asm/chacha-armv4.o.*obj/third_party/boringssl/boringssl_asm/sha512-armv4.o ##g' $file_name
}

add_openssl_libs() {
  local file_name=$1
  echo Add openssl lib to $file_name
  #Append openssl library path.
  has_ssl_ld=`grep openssl\/ios\/lib $file_name`
  if [ -z "$has_ssl_ld" ];then
    sed -i '' -e '/ldflags =/s/$/ -L\/Users\/testtest\/hezhen\/mediasdk\/third_party\/openssl\/ios\/lib/' $file_name
  fi
  has_ssl_lib=`grep lssl $file_name`
  if [ -z "$has_ssl_lib" ];then
    sed -i '' -e 's/libs =/& -lssl -lcrypto /' $file_name
  fi
}

grep -r boringssl . | grep -v -e boringssl.ninja -e boringssl_asm.ninja -e boringssl_crypto_tests_arch_executable.ninja -e boringssl_crypto_tests_arch_executable_sources.ninja -e boringssl_ssl_tests_arch_executable.ninja -e boringssl_ssl_tests_arch_executable_sources.ninja | grep ninja | awk -F : '{print $1}' | sort | uniq | while read line
do
  patch $line
done

fs=('./examples/stun_prober.ninja' './webrtc_nonparallel_tests_arch_executable.ninja' './rtc_tools/tools_unittests_arch_executable.ninja' './stats/rtc_stats_unittests_arch_executable.ninja' './modules/modules_unittests_arch_executable.ninja')
for f in ${fs[@]};do
  add_openssl_libs $f 
done

