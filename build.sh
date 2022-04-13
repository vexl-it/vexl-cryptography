#!/bin/bash
LIBFOLDER="$PWD/openssl"
TMPFOLDER="$LIBFOLDER/tmp"
OPENSSLREPO="$LIBFOLDER/repo"
PRODUCTFOLDER="$LIBFOLDER/lib"

OUTPUT_DARWIN_ARM64="$PRODUCTFOLDER/darwin-arm64"
OUTPUT_DARWIN_X86_64="$PRODUCTFOLDER/darwin-x86_64"
OUTPUT_DARWIN_SHARED="$PRODUCTFOLDER/darwin"
OUTPUT_IOS_SIMULATOR_X86_64="$PRODUCTFOLDER/ios-simulator-x86_64"
OUTPUT_IOS_SIMULATOR_ARM64="$PRODUCTFOLDER/ios-simulator-arm64"
OUTPUT_IOS_ARM64="$PRODUCTFOLDER/ios-arm64"

OUTPUT_LINUX_X86_64="$PRODUCTFOLDER/linux-x86_64"

TARGETS=(DARWIN_ARM64 DARWIN_X86_64, IOS_SIMULATOR_X86_64, IOS_SIMULATOR_ARM64, IOS_ARM64)
TARGETSCOUNT=${#TARGETS[@]}


printHelp() {
    echo "usage: ./build_openssl.sh [options]"
    echo "  options:"
    echo "    -d,   --darwin                 Creates universal openssl static library for macOS and iOS"
    echo "    -da,  --darwin-arm64           Creates arm openssl static library for macOS with apple silicon and iOS"
    echo "    -ds,  --darwin-x86_64          Creates x86_64 openssl static library for macOS with iOS simulator"
    echo "    -isa, --ios-simulator-arm64    Creates arm openssl static library for macOS with apple silicon and iOS"
    echo "    -isx, --ios-simulator-x86_64   Creates x86_64 openssl static library for macOS with iOS simulator"
    echo "    -ia,  --ios-arm64              Creates arm openssl static library for macOS with apple silicon and iOS"
}

build() {
    targetName=$1
    sslFolder="$TMPFOLDER/$1"

    case $1 in
        DARWIN_ARM64)
            outputPath=$OUTPUT_DARWIN_ARM64
            ;;
        DARWIN_X86_64)
            outputPath=$OUTPUT_DARWIN_X86_64
            ;;
        IOS_SIMULATOR_X86_64)
            outputPath=$OUTPUT_IOS_SIMULATOR_X86_64
            ;;
        IOS_SIMULATOR_ARM64)
            outputPath=$OUTPUT_IOS_SIMULATOR_ARM64
            ;;
        IOS_ARM64)
            outputPath=$OUTPUT_IOS_ARM64
            ;;
        *)
            echo "[OPENSSL] Did not match any target ($targetName)"
            return
            ;;
    esac

    if [ -d $outputPath ]; then
        echo "[OPENSSL] target $targetName already exists. Skipping."
        return
    fi

    cp -R $OPENSSLREPO $sslFolder
    cd $sslFolder

    case $1 in
        DARWIN_ARM64)
            echo "[OPENSSL] Building for darwing@arm64"
            ./Configure darwin64-arm64-cc \
                --prefix="$outputPath" \
                no-asm \
                no-shared
            ;;
        DARWIN_X86_64)
            echo "[OPENSSL] Building for darwing@x86_64"
            ./Configure darwin64-x86_64-cc \
                --prefix="$outputPath" \
                no-shared
            ;;
        IOS_SIMULATOR_X86_64)
            export CROSS_TOP="xcode-select --print-path/Platforms/iPhoneSimulator.platform/Developer"
            export CROSS_SDK="iPhoneSimulator.sdk"
            ./Configure iossimulator-xcrun "-arch x86_64" no-asm no-shared no-hw no-async --prefix="$outputPath"
            ;;
        IOS_SIMULATOR_ARM64)
            export CROSS_TOP="xcode-select --print-path/Platforms/iPhoneSimulator.platform/Developer"
            export CROSS_SDK="iPhoneSimulator.sdk"
            ./Configure iossimulator-xcrun "-arch arm64" no-asm no-shared no-hw no-async --prefix="$outputPath"
            ;;
        IOS_ARM64)
            export CC=clang;
            export CROSS_TOP=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer
            export CROSS_SDK=iPhoneOS.sdk
            export PATH="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin:$PATH"
            echo "[OPENSSL] Building for ios@arm64"
            ./Configure ios64-cross no-shared no-dso no-hw no-engine --prefix="$outputPath"
            ;;
        *)
            echo "[OPENSSL] Did not match any target ($targetName)"
            return
            ;;
    esac

    make
    make install_sw

    if [ ! -d $LIBFOLDER/include ]; then
        cp -R $outputPath/include $LIBFOLDER/include
    fi

    cd ..
}

mkdir -p $LIBFOLDER $TMPFOLDER
cd $LIBFOLDER

if [ -d $OPENSSLREPO ]; then
    echo "[OPENSSL] Using current OpenSSL repo"
else 
    git clone git://git.openssl.org/openssl.git $OPENSSLREPO
fi

built_darwin_arm64=false
built_darwin_x86_64=false

if [ "$#" = 0 ]; then 
    built_darwin_arm64=true
    built_darwin_x86_64=true
    for ((i=0; i < ${#TARGETS[@]}; i++)); do
        build ${TARGETS[i]} &
    done
else 
    while [[ $# -gt 0 ]]; do
        case $1 in
            -d|--darwin)
                build DARWIN_ARM64 $OUTPUT_DARWIN_ARM64
                build DARWIN_X86_64 $OUTPUT_DARWIN_X86_64
                built_darwin_arm64=true
                built_darwin_x86_64=true
                shift
                ;;
            -da|--darwin-arm64)
                build DARWIN_ARM64 $OUTPUT_DARWIN_ARM64
                built_darwin_arm64=true
                shift
                ;;
            -dx|--darwin-x86_64)
                build DARWIN_X86_64 $OUTPUT_DARWIN_X86_64
                built_darwin_x86_64=true
                shift
                ;;
            -isa|--ios-simulator-arm64)
                build IOS_SIMULATOR_ARM64  $OUTPUT_IOS_SIMULATOR_ARM64
                shift
                ;;
            -isx|--ios-simulator-x86_64)
                build IOS_SIMULATOR_X86_64  $OUTPUT_IOS_SIMULATOR_X86_64
                shift
                ;;
            -ia|--ios-arm64)
                build IOS_ARM64  $OUTPUT_IOS_ARM64
                shift
                ;;
            -lx|--darwin-arm64 )
                build LINUX_X86_64  $OUTPUT_LINUX_X86_64
                shift
                ;;
            -*|--*)
                printHelp
                exit 1
                ;;
            *)
                echo "[OPENSSL] Unknown argument $1"
                shift
                ;;
        esac
    done
fi

wait

echo "[OPENSSL] openssl cleanup"
rm -Rf $TMPFOLDER

