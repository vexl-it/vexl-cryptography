#!/bin/bash
LIBFOLDER="$PWD/openssl"
TMPFOLDER="$LIBFOLDER/tmp"
OPENSSLREPO="$LIBFOLDER/openssl-repo"

OUTPUT_DARWIN="$LIBFOLDER/darwin"
OUTPUT_DARWIN_ARM64="$OUTPUT_DARWIN/arm64"
OUTPUT_DARWIN_X86_64="$OUTPUT_DARWIN/x86_64"
OUTPUT_DARWIN_SHARED="$OUTPUT_DARWIN/shared"

mkdir -p $LIBFOLDER $TMPFOLDER
cd $LIBFOLDER

rm -Rf $OUTPUTFOLDER

git clone git://git.openssl.org/openssl.git $OPENSSLREPO

TARGETS=(DARWIN_ARM64 DARWIN_X86_64)
TARGETSCOUNT=${#TARGETS[@]}


build() {
    SSLFOLDER="$TMPFOLDER/$1"

    cp -R $OPENSSLREPO $SSLFOLDER
    cd $SSLFOLDER

    case $1 in
        DARWIN_ARM64)
            echo "[VEXL] Building for darwing@arm64"
            ./Configure darwin64-arm64-cc \
                --prefix="$OUTPUT_DARWIN_ARM64" \
                no-asm \
                no-shared
            ;;
        DARWIN_X86_64)
            echo "[VEXL] Building for darwing@x86_64"
            ./Configure darwin64-x86_64-cc \
                --prefix="$OUTPUT_DARWIN_X86_64"
            ;;
        *)
            echo "[VEXL] Did not match any target (arg: $1)"
            ;;
    esac

    make
    make install

    cd ..
}

for ((i=0; i < $TARGETSCOUNT; i++)); do
    build ${TARGETS[i]} &
done

wait

echo "[VEXL] Building shared darwing framework"
mkdir -p $OUTPUT_DARWIN_SHARED/lib/
lipo $OUTPUT_DARWIN_ARM64/lib/libssl.a $OUTPUT_DARWIN_X86_64/lib/libssl.a -create -output $OUTPUT_DARWIN_SHARED/lib/libssl.a
lipo $OUTPUT_DARWIN_ARM64/lib/libcrypto.a $OUTPUT_DARWIN_X86_64/lib/libcrypto.a -create -output $OUTPUT_DARWIN_SHARED/lib/libcrypto.a
cp -R $OUTPUT_DARWIN_ARM64/include $OUTPUT_DARWIN_SHARED/include

echo "[VEXL] cleanup"
rm -Rf $TMPFOLDER
