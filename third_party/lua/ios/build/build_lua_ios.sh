#!/bin/sh


###########################################################################
#  Change values here													  #
#				
VERSION="5.3.4"													      #
SDKVERSION=`xcrun -sdk iphoneos --show-sdk-version`														  #
#																		  #
###########################################################################
#																		  #
# Don't change anything under this line!								  #
#																		  #
###########################################################################


CURRENTPATH=`pwd`
ARCHS="armv7 arm64"
DEVELOPER=`xcode-select -print-path`

if [ ! -d "$DEVELOPER" ]; then
  echo "xcode path is not set correctly $DEVELOPER does not exist (most likely because of xcode > 4.3)"
  echo "run"
  echo "sudo xcode-select -switch <xcode path>"
  echo "for default installation:"
  echo "sudo xcode-select -switch /Applications/Xcode.app/Contents/Developer"
  exit 1
fi

case $DEVELOPER in  
     *\ * )
           echo "Your Xcode path contains whitespaces, which is not supported."
           exit 1
          ;;
esac

case $CURRENTPATH in  
     *\ * )
           echo "Your path contains whitespaces, which is not supported by 'make install'."
           exit 1
          ;;
esac

set -e
if [ ! -e lua-${VERSION}.tar.gz ]; then
	echo "Downloading lua-${VERSION}.tar.gz"
  curl -O https://www.lua.org/ftp/lua-${VERSION}.tar.gz
else
	echo "Using lua-${VERSION}.tar.gz"
fi

mkdir -p "${CURRENTPATH}/src"
mkdir -p "${CURRENTPATH}/bin"
mkdir -p "${CURRENTPATH}/../lib"

tar zxf lua-${VERSION}.tar.gz -C "${CURRENTPATH}/src"
cd "${CURRENTPATH}/src/lua-${VERSION}"

sed -ie "s!^PLATS=!PLATS= ios!" "Makefile"
cp -f "src/Makefile" "src/Makefilee"

for ARCH in ${ARCHS}
do
	if [[ "${ARCH}" == "i386" || "${ARCH}" == "x86_64" ]];
	then
		PLATFORM="iPhoneSimulator"
	else
		PLATFORM="iPhoneOS"
	fi
	
	export CROSS_TOP="${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer"
	export CROSS_SDK="${PLATFORM}${SDKVERSION}.sdk"
	export BUILD_TOOLS="${DEVELOPER}"

	echo "Building lua-${VERSION} for ${PLATFORM} ${SDKVERSION} ${ARCH}"
	echo "Please stand by..."

	# export CC="${BUILD_TOOLS}/usr/bin/gcc -arch ${ARCH}"
	mkdir -p "${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk"
	LOG="${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk/build-lua-${VERSION}.log"

  cp -f "src/Makefilee" "src/Makefile"

  echo "ios:\n\t\$(MAKE) \$(ALL_A) SYSCFLAGS='-DLUA_USE_LINUX' SYSLIBS='-Wl, -ldl -lreadline' CC='${BUILD_TOOLS}/usr/bin/gcc -arch ${ARCH}' MYCFLAGS='-isysroot ${CROSS_TOP}/SDKs/${CROSS_SDK} -miphoneos-version-min=7.0 -fembed-bitcode' LDFLAGS='-isysroot ${CROSS_TOP}/SDKs/${CROSS_SDK} -miphoneos-version-min=7.0'" >> "src/Makefile"

  if [ "$1" == "verbose" ];
	then
		make ios
	else
		make ios >> "${LOG}" 2>&1
	fi
	
	if [ $? != 0 ];
  then 
  	echo "Problem while make - Please check ${LOG}"
    exit 1
  fi
    
  set -e
	cp -f "${CURRENTPATH}/src/lua-${VERSION}/src/liblua.a" "${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk"
	make clean >> "${LOG}" 2>&1
done

mv "src/Makefilee" "src/Makefile"


echo "Build library..."
lipo -create ${CURRENTPATH}/bin/iPhoneOS${SDKVERSION}-armv7.sdk/liblua.a ${CURRENTPATH}/bin/iPhoneOS${SDKVERSION}-arm64.sdk/liblua.a -output ${CURRENTPATH}/../lib/liblua.a

mkdir -p ${CURRENTPATH}/../include
cd src
cp -f lua.h luaconf.h lualib.h lauxlib.h lua.hpp ${CURRENTPATH}/../include/
echo "Building done."
echo "Cleaning up..."
rm -fr ${CURRENTPATH}/src
rm -fr ${CURRENTPATH}/bin
echo "Done."


