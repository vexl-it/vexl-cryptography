#!/bin/bash
LIBFOLDER="$PWD/openssl"
TMPFOLDER="$LIBFOLDER/tmp"
OPENSSLREPO="$LIBFOLDER/repo"
PRODUCTFOLDER="$LIBFOLDER/lib"

OUTPUT_DARWIN_ARM64="$PRODUCTFOLDER/darwin-arm64"
OUTPUT_DARWIN_X86_64="$PRODUCTFOLDER/darwin-x86_64"
OUTPUT_DARWIN_SHARED="$PRODUCTFOLDER/darwin"

OUTPUT_LINUX_X86_64="$PRODUCTFOLDER/linux-x86_64"

TARGETS=(DARWIN_ARM64 DARWIN_X86_64, LINUX_X86_64)
TARGETSCOUNT=${#TARGETS[@]}


printHelp() {
    echo "usage: ./build_openssl.sh [options]"
    echo "  options:"
    echo "    -d, --darwin            Creates universal openssl static library for macOS and iOS"
    echo "    -da, --darwin-arm64     Creates arm openssl static library for macOS with apple silicon and iOS"
    echo "    -ds, --darwin-x86_64    Creates x86_64 openssl static library for macOS with iOS simulator"
    echo "    -lx, --linux-x86_64    Creates x86_64 openssl static library for macOS with iOS simulator"
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
        LINUX_X86_64)
            outputPath=$OUTPUT_LINUX_X86_64
            ;;
        *)
            echo "[VEXL] Did not match any target ($targetName)"
            return
            ;;
    esac

    if [ -d $outputPath ]; then
        echo "[VEXL] target $targetName already exists. Skipping."
        return
    fi

    cp -R $OPENSSLREPO $sslFolder
    cd $sslFolder

    case $1 in
        DARWIN_ARM64)
            echo "[VEXL] Building for darwing@arm64"
            ./Configure darwin64-arm64-cc \
                --prefix="$outputPath" \
                no-asm \
                no-shared
            ;;
        DARWIN_X86_64)
            echo "[VEXL] Building for darwing@x86_64"
            ./Configure darwin64-x86_64-cc \
                --prefix="$outputPath" \
                no-shared
            ;;
        LINUX_X86_64)
            echo "[VEXL] Building for linux@x86_64"
            ./config linux-x86_64 \
                --prefix="$outputPath" \
                no-shared
            ;;
        *)
            echo "[VEXL] Did not match any target ($targetName)"
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

if [ -d $LIBFOLDER/include ]; then
    echo "[VEXL] Using current OpenSSL repo"
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
            -lx|--linux-x86_64)
                build LINUX_X86_64  $OUTPUT_LINUX_X86_64
                shift
                ;;
            -*|--*)
                printHelp
                exit 1
                ;;
            *)
                shift
                ;;
        esac
    done
fi

wait

if [ "$built_darwin_arm64" = true -a "$built_darwin_x86_64" = true -a ! -d $OUTPUT_DARWIN_SHARED ] ; then
    echo "[VEXL] Building shared darwing framework"
    mkdir -p $OUTPUT_DARWIN_SHARED/lib/
    lipo $OUTPUT_DARWIN_ARM64/lib/libssl.a $OUTPUT_DARWIN_X86_64/lib/libssl.a -create -output $OUTPUT_DARWIN_SHARED/lib/libssl.a
    lipo $OUTPUT_DARWIN_ARM64/lib/libcrypto.a $OUTPUT_DARWIN_X86_64/lib/libcrypto.a -create -output $OUTPUT_DARWIN_SHARED/lib/libcrypto.a
    cp -R $OUTPUT_DARWIN_ARM64/include $OUTPUT_DARWIN_SHARED/include
fi

echo "[VEXL] openssl cleanup"
rm -Rf $TMPFOLDER

