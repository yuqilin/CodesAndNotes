#!/bin/sh

#############################
# check Android SDK
#############################
if [ -z "${ANDROID_SDK}" ]; then
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    echo "+     You have to export ANDROID_SDK at first.    	     	     +"
    echo "+     They should point to your Android SDK root directories.      +"
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    echo ""
    exit 1
fi

ADB_TOOL="$ANDROID_SDK/platform-tools/adb"
LOGDIR="/tmp/logcat"
LOGDATE=`date +%Y%m%d%H%M%S`
FILENAME=${LOGDATE}.log

#if [ ! -z $1 ]; then
#    FILENAME=$1
#fi

mkdir -p ${LOGDIR}

LOGFILE="${LOGDIR}/${FILENAME}"

echo "adb logcat => ${LOGFILE}"
${ADB_TOOL} logcat -c
${ADB_TOOL} logcat -v threadtime *:V | tee ${LOGFILE} | egrep -i "$1"
