#!/bin/bash

set -e
VERSION=5.3.4
BASE_DIR=`pwd`
INCLUDE_DIR="$BASE_DIR/../include"
LIB_DIR="$BASE_DIR/../lib"

LUA_NAME="lua-$VERSION"
LUA_FILE="$LUA_NAME.tar.gz"
LUA_URL="http://www.lua.org/ftp/$LUA_FILE"
LUA_PATH="$BASE_DIR/$LUA_FILE"

ROOT_PATH="$BASE_DIR/$LUA_NAME"
SRC_PATH="$ROOT_PATH/src"
MAKEFILE_PATH="$SRC_PATH/Makefile"
MAKEFILE_PATH_BAK="$SRC_PATH/Makefile.bak"

_ANDROID_API="android-16"

# Retrieve lua tarbal if needed
#if [ ! -e "$LUA_PATH" ]; then
#	curl "$LUA_URL" -o "$LUA_PATH"
#fi

tar vxf "$LUA_PATH" -C $BASE_DIR

#Backup original Makefile
cp $MAKEFILE_PATH $MAKEFILE_PATH_BAK

#Modify Makefile for setting build tool
sed -i 's/^CC=/#&/g' $MAKEFILE_PATH  #Ignore default CC 
sed -i 's/^AR=/#&/g' $MAKEFILE_PATH  #Ignore default AR
sed -i 's/^RANLIB=/#&/g' $MAKEFILE_PATH  #Ignore default RANLIB
sed -i '/^ALL_T=/c ALL_T=$(LUA_A)' $MAKEFILE_PATH  #Only compile liblua.a

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
    . $BASE_DIR/setenv-android-mod.sh

    echo "CROSS COMPILE ENV : $CROSS_COMPILE"
    cd $SRC_PATH

    make clean 

    #Backup current Makefile    
    TMP="$MAKEFILE_PATH.1"
    cp $MAKEFILE_PATH $TMP
    
    xCFLAGS="-mandroid -I$ANDROID_DEV/include -L$ANDROID_DEV/$xLib --sysroot=$ANDROID_SYSROOT -lstd++"
    if [ $_ANDROID_API == "android-9" ]; then
        xCFLAGS+=" -Dlua_getlocaledecpoint\\\\(\\\\)=\\\\(\\\\\'.\\\\'\\\\)"
    fi

    sed -i "s#^CFLAGS=.*#&$xCFLAGS#g" $MAKEFILE_PATH    #Modify Makefile for adding platform specified configuration.
    
    export CC="$ANDROID_TOOLCHAIN/${CROSS_COMPILE}gcc"  #Set c compiler
    export AR="$ANDROID_TOOLCHAIN/${CROSS_COMPILE}ar rcu"
    export RANLIB="$ANDROID_TOOLCHAIN/${CROSS_COMPILE}ranlib" 

    echo Building ${arch} ${MYCFLAGS}
    make linux
    echo Build ${arch} Done

    cp liblua.a $LIB_DIR/${arch}/liblua.a
    
    #Restore Makefile
    mv $TMP $MAKEFILE_PATH    

    cd $BASE_DIR 
done
#rm -rf $LUA_NAME
mv $MAKEFILE_PATH_BAK $MAKEFILE_PATH 
exit 0
