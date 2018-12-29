#!/bin/bash

BEE_VERSION=2.0.1.1
FILE=./src/api/bee/base/bee_version.h

[ -f $FILE ] && {
rm -fr $FILE
}

BUILD_BRANCH=`git branch -a | grep '*' | awk '{print $2}'`
BUILD_COMMIT=`git log | head -n3 | grep commit  | awk '{print $2}'`
BUILD_TIME=`git log | head -n3 | grep Date  | awk -F "Date:" '{print $2}' | sed 's/^[ \t]*//g'`
BUILD_BY=`whoami`

echo '//Build version info, DO NOT commit this file. ' >> $FILE
echo '#ifndef __BEE_VERSION_H__' >> $FILE
echo '#define __BEE_VERSION_H__' >> $FILE
echo '' >> $FILE
echo "#define BEE_VERSION \"$BEE_VERSION\"" >> $FILE
echo "#define BUILD_BRANCH \"$BUILD_BRANCH\"" >> $FILE
echo "#define BUILD_COMMIT \"$BUILD_COMMIT\"" >> $FILE
echo "#define BUILD_TIME \"$BUILD_TIME\"" >> $FILE
echo "#define BUILD_BY \"$BUILD_BY\"" >> $FILE
echo '' >> $FILE
echo '#endif //ifndef __BEE_VERSION_H__' >> $FILE
