#!/bin/bash
#
# http://wiki.openssl.org/index.php/Android
#
set -e
VERSION=1.0.2j
BASE_DIR=`pwd`
INCLUDE_DIR="$BASE_DIR/../include"
LIB_DIR="$BASE_DIR/../lib"

OPENSSL_NAME="openssl-$VERSION"
OPENSSL_FILE="$OPENSSL_NAME.tar.gz"
OPENSSL_URL="http://www.openssl.org/source/$OPENSSL_FILE"
OPENSSL_PATH="./$OPENSSL_FILE"
_ANDROID_API="android-16"

# Retrieve OpenSSL tarbal if needed
#if [ ! -e "$OPENSSL_PATH" ]; then
#	curl "$OPENSSL_URL" -o "$OPENSSL_PATH"
#fi

tar vxf "$OPENSSL_PATH" -C $BASE_DIR

archs=(armeabi armeabi-v7a arm64-v8a mips mips64 x86 x86_64)

for arch in ${archs[@]}; do
    xLIB="/lib"
    case ${arch} in
        "armeabi")
            _ANDROID_TARGET_SELECT=arch-arm
            _ANDROID_ARCH=arch-arm
            _ANDROID_EABI=arm-linux-androideabi-4.9
            configure_platform="android" ;;
        "armeabi-v7a")
            _ANDROID_TARGET_SELECT=arch-arm
            _ANDROID_ARCH=arch-arm
            _ANDROID_EABI=arm-linux-androideabi-4.9
            configure_platform="android-armv7" ;;
        "arm64-v8a")
            _ANDROID_TARGET_SELECT=arch-arm64-v8a
            _ANDROID_ARCH=arch-arm64
            _ANDROID_EABI=aarch64-linux-android-4.9
            xLIB="/lib64"
	    _ANDROID_API="android-21"
            configure_platform="linux-generic64 -DB_ENDIAN" ;;
        "mips")
            _ANDROID_TARGET_SELECT=arch-mips
            _ANDROID_ARCH=arch-mips
            _ANDROID_EABI=mipsel-linux-android-4.9
            configure_platform="android -DB_ENDIAN" ;;
        "mips64")
            _ANDROID_TARGET_SELECT=arch-mips64
            _ANDROID_ARCH=arch-mips64
            _ANDROID_EABI=mips64el-linux-android-4.9
            xLIB="/lib64"
	    _ANDROID_API="android-21"
            configure_platform="linux-generic64 -DB_ENDIAN" ;;
        "x86")
            _ANDROID_TARGET_SELECT=arch-x86
            _ANDROID_ARCH=arch-x86
            _ANDROID_EABI=x86-4.9
            configure_platform="android-x86" ;;
        "x86_64")
            _ANDROID_TARGET_SELECT=arch-x86_64
            _ANDROID_ARCH=arch-x86_64
            _ANDROID_EABI=x86_64-4.9
            xLIB="/lib64"
	    _ANDROID_API="android-21"
            configure_platform="linux-generic64" ;;
        *)
            configure_platform="linux-elf" ;;
    esac

    mkdir -p $LIB_DIR/${arch}
    . ./setenv-android-mod.sh

    echo "CROSS COMPILE ENV : $CROSS_COMPILE"
    cd $OPENSSL_NAME
    xCFLAGS="-fPIC -DOPENSSL_PIC -DDSO_DLFCN -DHAVE_DLFCN_H -mandroid -I$ANDROID_DEV/include -L$ANDROID_DEV/$xLib -O3 -fomit-frame-pointer -Wall --sysroot=$ANDROID_SYSROOT" 
    perl -pi -e 's/install: all install_docs install_sw/install: install_docs install_sw/g' Makefile.org
    ./Configure $configure_platform shared no-threads no-asm no-zlib no-ssl2 no-ssl3 no-comp no-hw no-engine $xCFLAGS
    # patch SONAME

    perl -pi -e 's/SHLIB_EXT=\.so\.\$\(SHLIB_MAJOR\)\.\$\(SHLIB_MINOR\)/SHLIB_EXT=\.so/g' Makefile
    perl -pi -e 's/SHARED_LIBS_LINK_EXTS=\.so\.\$\(SHLIB_MAJOR\) \.so//g' Makefile
    # quote injection for proper SONAME, fuck...
    perl -pi -e 's/SHLIB_MAJOR=1/SHLIB_MAJOR=`/g' Makefile
    perl -pi -e 's/SHLIB_MINOR=0.0/SHLIB_MINOR=`/g' Makefile
    make clean
    LOG_FILE="$BASE_DIR/$ANDROID_API-$ARCH.log"

    if [ ! -e "$LOG_FILE" ]; then
      touch $LOG_FILE
    fi

    echo Building ${arch}
    make depend >> "$LOG_FILE"
    make all >> "$LOG_FILE"
    
    if [ ! -d "$INCLUDE_DIR/openssl" ]; then
    	mkdir -p $INCLUDE_DIR/openssl
    fi
    n=`ls $INCLUDE_DIR/openssl | wc -l`
    if [ $n -eq 0 ] 
    then
	sed -i '' include/openssl/*  #Replace soft link with original file trick.
        cp -R include/openssl $INCLUDE_DIR
    fi

    cp libcrypto.a $LIB_DIR/${arch}/libcrypto.a
    cp libssl.a $LIB_DIR/${arch}/libssl.a
    echo Build ${arch} Done
    
    cd ..
done
rm -rf $OPENSSL_NAME
exit 0
