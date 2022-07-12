#!/bin/bash
LIBFOLDER="$PWD/../openssl"
TMPFOLDER="$LIBFOLDER/tmp"
OPENSSLREPO="$LIBFOLDER/repo"
PRODUCTFOLDER="$LIBFOLDER/lib"

OUTPUT_DARWIN_ARM64="$PRODUCTFOLDER/darwin-arm64"
OUTPUT_DARWIN_X86_64="$PRODUCTFOLDER/darwin-x86_64"
OUTPUT_DARWIN_SHARED="$PRODUCTFOLDER/darwin"
OUTPUT_IOS_SIMULATOR_X86_64="$PRODUCTFOLDER/ios-simulator-x86_64"
OUTPUT_IOS_SIMULATOR_ARM64="$PRODUCTFOLDER/ios-simulator-arm64"
OUTPUT_IOS_ARM64="$PRODUCTFOLDER/ios-arm64"

OUTPUT_ANDROID_ARMV8="$PRODUCTFOLDER/android-armv8"
OUTPUT_ANDROID_ARMV7="$PRODUCTFOLDER/android-armv7"
OUTPUT_ANDROID_X86="$PRODUCTFOLDER/android-x86"
OUTPUT_ANDROID_X86_64="$PRODUCTFOLDER/android-x86_64"

OUTPUT_LINUX_ARM64="$PRODUCTFOLDER/linux-arm64"
OUTPUT_LINUX_X86_64="$PRODUCTFOLDER/linux-x86_64"

OUTPUT_WINDOWS_X86_64="$PRODUCTFOLDER/windows-x86_64"

TARGETS=(DARWIN_ARM64 DARWIN_X86_64, IOS_SIMULATOR_X86_64, IOS_SIMULATOR_ARM64, IOS_ARM64, ANDROID_ARMV8, ANDROID_ARMV7, ANDROID_X86, ANDROID_X86_64, LINUX_ARM64, LINUX_X86_64, WINDOWS_X86_64)
TARGETSCOUNT=${#TARGETS[@]}
CURRENTUSER="$(whoami)"


printHelp() {
    echo "usage: ./build_openssl.sh [options]"
    echo "  options:"
    echo "    -da,  --darwin-arm64           Creates arm openssl static library for macOS with Apple silicon SoC"
    echo "    -ds,  --darwin-x86_64          Creates x86_64 openssl static library for macOS with Intel CPUs"
    echo "    -isa, --ios-simulator-arm64    Creates arm openssl static library for iOS simulator running on Apple silicon SoC"
    echo "    -isx, --ios-simulator-x86_64   Creates x86_64 openssl static library for iOS simulator running on Intel CPU"
    echo "    -ia,  --ios-arm64              Creates arm openssl static library for iOS"
    echo "    -aav7,  --android-armv7        Creates openssl static library for android devices running linux with 32-bit ARMv7 SoC"
    echo "    -aav8,  --android-armv8        Creates openssl static library for android devices running linux with 64-bit ARMv8 or ARM64 SoC"
    echo "    -ax64,  --android-x86_64       Creates openssl static library for android devices running on x86_64 CPU"
    echo "    -ax86,  --android-x86          Creates openssl static library for android devices running on x86 CPU"
    echo "    -la,  --linux-arm64            Creates openssl static library for android devices running linux with 64-bit ARMv8 or ARM64 SoC"
    echo "    -lx,  --linux-x86_64           Creates openssl static library for android devices running on x86_64 CPU"
    echo "    -wx,  --windows-x86_64         Creates openssl static library for android devices running on x86_64 CPU"
}

build() {
    targetName=$1
    sslFolder="$TMPFOLDER/$1"

    outputPath=$2

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
            echo "[OPENSSL] Building for ios_sim@x86_64"
            export CROSS_TOP=/Applications/Xcode-12.0.0.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer
            export CROSS_SDK=iPhoneSimulator.sdk
            ./Configure iossimulator-xcrun "-arch x86_64 -mios-simulator-version-min=14.0" no-asm no-shared no-hw --prefix="$outputPath" "-isysroot /Applications/Xcode-12.0.0.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator14.0.sdk"
            ;;
        IOS_SIMULATOR_ARM64)
            echo "[OPENSSL] Building for ios_sim@arm64"
            export CROSS_TOP=/Applications/Xcode-12.0.0.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer
            export CROSS_SDK=iPhoneSimulator.sdk
            ./Configure iossimulator-xcrun "-arch arm64 -mios-simulator-version-min=14.0" no-asm no-shared no-hw --prefix="$outputPath" "-isysroot /Applications/Xcode-12.0.0.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator14.0.sdk"
            ;;
        IOS_ARM64)
            echo "[OPENSSL] Building for ios@arm64"
            export CC=clang;
            export CROSS_TOP=/Applications/Xcode-12.0.0.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer
            export CROSS_SDK=iPhoneOS.sdk
            ./Configure ios64-cross "-arch arm64 -fembed-bitcode -miphoneos-version-min=14.0" no-asm no-shared no-hw --prefix="$outputPath" "-isysroot /Applications/Xcode-12.0.0.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS14.0.sdk"
            ;;
        ANDROID_ARMV8)
            echo "[OPENSSL] Building for android@arm64"
            export ANDROID_NDK_ROOT="/Users/$CURRENTUSER/Library/Android/sdk/ndk/24.0.8215888"
            export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/darwin-x86_64/bin:$PATH
            ./Configure android-arm64 --prefix="$outputPath" no-shared
            ;;
        ANDROID_ARMV7)
            export ANDROID_NDK_ROOT="/Users/$CURRENTUSER/Library/Android/sdk/ndk/24.0.8215888"
            export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/darwin-x86_64/bin:$PATH
            echo "[OPENSSL] Building for android@armv7"
            ./Configure android-armeabi --prefix="$outputPath" no-shared
            sed -i -e 's/CC=$(CROSS_COMPILE)32-clang/CC=$(CROSS_COMPILE)armv7a-linux-androideabi32-clang/g' ./Makefile
            ;;
        ANDROID_X86_64)
            export ANDROID_NDK_ROOT="/Users/$CURRENTUSER/Library/Android/sdk/ndk/24.0.8215888"
            export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/darwin-x86_64/bin:$PATH
            echo "[OPENSSL] Building for android@x86_64"
            ./Configure android-x86_64 --prefix="$outputPath" no-shared
            ;;
        ANDROID_X86)
            export ANDROID_NDK_ROOT="/Users/$CURRENTUSER/Library/Android/sdk/ndk/24.0.8215888"
            export PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/darwin-x86_64/bin:$PATH
            echo "[OPENSSL] Building for android@x86"
            ./Configure android-x86 --prefix="$outputPath" no-shared
            ;;
        LINUX_ARM64)
            echo "[OPENSSL] Building for linux@arm64"
            ./config linux-aarch64 --prefix="$outputPath" no-shared
            ;;
        LINUX_X86_64)
            echo "[OPENSSL] Building for linux@x86_64"
            ./config linux-x86_64 --prefix="$outputPath" no-shared
            ;;
        WINDOWS_X86_64)
            echo "[OPENSSL] Building for windows@x86_64"
            ./Configure mingw64 --cross-compile-prefix=x86_64-w64-mingw32- --prefix="$outputPath" no-shared
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

if [ "$#" = 0 ]; then
    printHelp
    exit
else 
    while [[ $# -gt 0 ]]; do
        case $1 in
            -da|--darwin-arm64)
                build DARWIN_ARM64 $OUTPUT_DARWIN_ARM64
                shift
                ;;
            -dx|--darwin-x86_64)
                build DARWIN_X86_64 $OUTPUT_DARWIN_X86_64
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
            -aav7|--android-armv7 )
                build ANDROID_ARMV7  $OUTPUT_ANDROID_ARMV7
                shift
                ;;
            -aav8|--android-armv8 )
                build ANDROID_ARMV8  $OUTPUT_ANDROID_ARMV8
                shift
                ;;
            -ax64|--android-x86_64 )
                build ANDROID_X86_64  $OUTPUT_ANDROID_X86_64
                shift
                ;;
            -ax86|--android-x86 )
                build ANDROID_X86  $OUTPUT_ANDROID_X86
                shift
                ;;
            -la|--linux-arm64 )
                build LINUX_ARM64  $OUTPUT_LINUX_ARM64
                shift
                ;;
            -lx|--linux-x86_64 )
                build LINUX_X86_64  $OUTPUT_LINUX_X86_64
                shift
                ;;
            -wx|--windows-x86_64 )
                build WINDOWS_X86_64  $OUTPUT_WINDOWS_X86_64
                shift
                ;;
            -h|--help )
                printHelp
                exit 0
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

