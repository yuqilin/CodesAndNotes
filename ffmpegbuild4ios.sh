#!/bin/sh

set -e

# ffmpeg SDK version
FFMPEG_VERSION="2.3"
SDK_VERSION="7.1"

# mini supprted SDK version
MINSDKVERSION="7.0"

SRCDIR=$(pwd)
BUILDDIR="${SRCDIR}/build/ios"
mkdir -p ${BUILDDIR}

# xcode install path
DEVELOPER=`xcode-select -print-path`

#--cc="${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/usr/bin/gcc" 

# archs
ARCHS="armv7 armv7s i386"
for ARCH in ${ARCHS}
do
    if [ "${ARCH}" == "i386" ];
    then
        PLATFORM="iPhoneSimulator"
        EXTRA_CFLAGS="-arch i386"
        EXTRA_LDFLAGS="-arch i386 -mfpu=neon"
        EXTRA_CONFIG="--arch=i386 --cpu=i386"
    else
        PLATFORM="iPhoneOS"
        EXTRA_CFLAGS="-arch ${ARCH} -mfloat-abi=softfp"
        EXTRA_LDFLAGS="-arch ${ARCH} -mfpu=neon -mfloat-abi=softtp"
        EXTRA_CONFIG="--arch=arm --cpu=cortex-a9 --disable-armv5te"
    fi

    #make clean

    ./configure --target-os=darwin \
    --prefix="${BUILDDIR}/${ARCH}" \
        --disable-doc \
        --disable-programs \
        --disable-ffmpeg \
        --disable-ffplay \
        --disable-ffprobe \
        --disable-ffserver \
        --enable-cross-compile \
        --enable-static \
        --disable-shared \
        --enable-debug \
        --enable-decoders \
        --enable-demuxers \
        --disable-muxers \
        --disable-swscale-alpha \
        --enable-small \
        --disable-vaapi \
        --disable-vdpau \
        --disable-dxva2 \
        --disable-bsfs \
        --disable-filters \
        --disable-gpl \
        --disable-devices \
        --disable-postproc \
        --disable-avdevice \
        --disable-avfilter \
        --disable-armv6 \
        --disable-armv6t2 \
        --enable-network \
        --enable-indevs \
        --enable-protocols \
        --enable-bsf=h264_mp4toannexb \
        --enable-pthreads \
        ${EXTRA_CONFIG} \
        --cc="${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang" \
        --sysroot="${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDK_VERSION}.sdk" \
        --extra-cflags="-miphoneos-version-min=${MINSDKVERSION} ${EXTRA_CFLAGS}" \
        --extra-ldflags="-miphoneos-version-min=${MINSDKVERSION} ${EXTRA_LDFLAGS} -isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDK_VERSION}.sdk"

    make clean
    make -j8
    make install

done

# create fat library
mkdir -p ${BUILDDIR}/universal/lib
cd ${BUILDDIR}/armv7/lib

for file in *.a
do
    cd ${BUILDDIR}
    xcrun -sdk iphoneos lipo -output universal/lib/$file -create -arch armv7 armv7/lib/$file -arch armv7s armv7s/lib/$file -arch i386 i386/lib/$file
    echo "Universal $file created..."

done

cp -r ${BUILDDIR}/armv7/include ${BUILDDIR}/universal/

echo "Done."

