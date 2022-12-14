# folder structure:
# 
# ./src - source files for the library
# ./tests - source files for tests 
# ./product - outputfolder where everything is compiled
#    |-<cpu architectures>
#    |   |-lib/libvc.a - the static library
#    |   |_inclure/vc/* - corresponding header files
#    |
#    |_tests/test - binary which runs the tests, build for current cpu architecture
# 

ARM=arch -arm64
X86=arch -x86_64

CC=gcc
AR=ar
LIBTOOL=libtool

TMPFOLDER=tmp
PRODUCTFOLDER=product
ANDROIDFOLDER=$(PRODUCTFOLDER)/android/vexl-crypto.framework
SRCFOLDER=src
TESTFOLDER=tests
OPENSSLFOLDER=../openssl
SSLINCLUDE=$(OPENSSLFOLDER)/include
SSLLIB=$(OPENSSLFOLDER)/lib
TESTBIN=$(PRODUCTFOLDER)/$(TESTFOLDER)/test

# Compiler flags
CFLAGS=-MP -MD -g -w -fPIC
LCFLAGS= $(CFLAGS) -DBUILD_FOR_LIBRARY

# ARCH variables
APPLE_ARCHITECTURES=darwin-x86_64 darwin-arm64 ios-simulator-x86_64 ios-simulator-arm64 ios-arm64
ANDROID_ARCHITECTURES=android-armv8 android-armv7 android-x86 android-x86_64
DOCKER_LINUX_ARCHITECTURES=docker-linux-arm64 docker-linux-x86_64
LINUX_ARCHITECTURES=linux-arm64 linux-x86_64
WINDOWS_ARCHITECTURES=windows-x86_64
ARCHITECTURES=$(APPLE_ARCHITECTURES) $(APPLE_ARCHITECTURES) $(DOCKER_LINUX_ARCHITECTURES) $(WINDOWS_ARCHITECTURES)

ARCHOFOLDERS=$(foreach ARCH,$(ARCHITECTURES),$(TMPFOLDER)/$(ARCH))
CURRENTARCH=$(shell uname | tr A-Z a-z)-$(shell uname -m | tr A-Z a-z)

MACOS_SDK=$(shell xcrun --sdk macosx --show-sdk-path)
IOS_SIM_SDK=/Applications/Xcode-12.0.0.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator14.0.sdk
IOS_SDK=/Applications/Xcode-12.0.0.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS14.0.sdk

ANDROID_NDK_ROOT=~/Library/Android/sdk/ndk/24.0.8215888
ANDROID_ARM64_CROSS_CC=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/darwin-x86_64/bin/aarch64-linux-android32-clang
ANDROID_ARMv7_CROSS_CC=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/darwin-x86_64/bin/armv7a-linux-androideabi32-clang
ANDROID_X86_64_CROSS_CC=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/darwin-x86_64/bin/x86_64-linux-android32-clang
ANDROID_X86_CROSS_CC=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/darwin-x86_64/bin/i686-linux-android32-clang
ANDROID_AR=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/darwin-x86_64/bin/llvm-ar

WINDOWS_CROSS_CC=x86_64-w64-mingw32-gcc
# OpenSSL builds
OPENSSL_APPLE_TARGETS=$(foreach ARCH, $(APPLE_ARCHITECTURES), build-openssl-$(ARCH))
OPENSSL_ANDROID_TARGETS=$(foreach ARCH, $(ANDROID_ARCHITECTURES), build-openssl-$(ARCH))
OPENSSL_LINUX_TARGETS=$(foreach ARCH, $(LINUX_ARCHITECTURES), build-openssl-$(ARCH))
OPENSSL_WINDOWS_TARGETS=$(foreach ARCH, $(WINDOWS_ARCHITECTURES), build-openssl-$(ARCH))
OPENSSLTARGETS=$(OPENSSL_APPLE_TARGETS) $(OPENSSL_ANDROID_TARGETS) $(OPENSSL_LINUX_TARGETS) $(OPENSSL_WINDOWS_TARGETS)

# Library files
SRCFOLDER=src
CFILES=$(wildcard $(SRCFOLDER)/*.c) $(wildcard $(SRCFOLDER)/**/*.c)
OFILES=\
$(foreach ARCHFOLDER,$(ARCHOFOLDERS),\
	$(foreach CFILE, $(CFILES), \
		$(patsubst %.c,%.o,$(ARCHFOLDER)/$(CFILE)) \
	)\
)

# Tests files
TESTCFILES=$(foreach D,$(TESTFOLDER),$(wildcard $(D)/*.c))
TESTOFILES=\
$(foreach CFILE, $(TESTCFILES), \
	$(patsubst %.c,%.o,$(TMPFOLDER)/$(CURRENTARCH)/$(CFILE)) \
)

# clean variables
ALLCFILES= $(CFILES) $(TESTCFILES)
DEPFILES=$(patsubst %.c,%.d,$(ALLCFILES))
-include $(DEPFILES)

build-current: build-openssl-$(CURRENTARCH) $(CURRENTARCH) test

all: apple android linux windows test

$(OPENSSLTARGETS):
	./build.sh --$(@:build-openssl-%=%)

apple: $(OPENSSL_APPLE_TARGETS) $(APPLE_ARCHITECTURES)
	@mkdir -p $(PRODUCTFOLDER)/darwin-universal/lib $(PRODUCTFOLDER)/ios-simulator-universal/lib
	@cp -R $(PRODUCTFOLDER)/darwin-arm64/include $(PRODUCTFOLDER)/darwin-universal/include
	@cp -R $(PRODUCTFOLDER)/ios-simulator-arm64/include $(PRODUCTFOLDER)/ios-simulator-universal/include
	lipo -create $(PRODUCTFOLDER)/darwin-arm64/lib/libvc.a $(PRODUCTFOLDER)/darwin-x86_64/lib/libvc.a -output $(PRODUCTFOLDER)/darwin-universal/lib/libvc.a
	lipo -create $(PRODUCTFOLDER)/ios-simulator-arm64/lib/libvc.a $(PRODUCTFOLDER)/ios-simulator-x86_64/lib/libvc.a -output $(PRODUCTFOLDER)/ios-simulator-universal/lib/libvc.a
	$(eval LIBFARCH := darwin-universal ios-simulator-universal ios-arm64)
	$(eval LIBS := $(foreach ARCH, $(LIBFARCH), \
		-library $(PRODUCTFOLDER)/$(ARCH)/lib/libvc.a -headers $(PRODUCTFOLDER)/$(ARCH)/include \
	))
	xcodebuild -create-xcframework $(LIBS) -output $(PRODUCTFOLDER)/apple/vexl-crypto.xcframework
	zip -q -r $(PRODUCTFOLDER)/apple/vexl_crypto_ios_xcframework.zip $(PRODUCTFOLDER)/apple/vexl-crypto.xcframework

android: $(OPENSSL_ANDROID_TARGETS) $(ANDROID_ARCHITECTURES)
	@mkdir -p $(ANDROIDFOLDER)/include/vc
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(ANDROIDFOLDER)/include/vc
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(ANDROIDFOLDER)/include/vc
	cd $(ANDROIDFOLDER)/.. && zip -r vexl_crypto_android_frameworks.zip vexl-crypto.framework

linux: $(DOCKER_LINUX_ARCHITECTURES)
	@mkdir -p $(PRODUCTFOLDER)/linux
	cd $(PRODUCTFOLDER) && zip -r linux/vexl_crypto_linux_frameworks.zip linux-arm64 linux-x86_64

windows: $(WINDOWS_ARCHITECTURES)
	@mkdir -p $(PRODUCTFOLDER)/windows
	cd $(PRODUCTFOLDER) && zip -r windows/vexl_crypto_windows_frameworks.zip windows-x86_64

darwin-x86_64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/darwin-x86_64/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(X86) $(AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc_.a $^
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc 
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc 
	$(X86) $(LIBTOOL) -static -o $(PRODUCTFOLDER)/$@/lib/libvc.a $(PRODUCTFOLDER)/$@/lib/libvc_.a $(SSLLIB)/$@/lib/libcrypto.a $(SSLLIB)/$@/lib/libssl.a
	@rm $(PRODUCTFOLDER)/$@/lib/libvc_.a

$(TMPFOLDER)/darwin-x86_64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(X86) $(CC) -I$(SSLINCLUDE) $(LCFLAGS) -isysroot $(MACOS_SDK) -c -o $@ $< -target macos-x86_64

darwin-arm64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/darwin-arm64/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(ARM) $(AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc_.a $^
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc 
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc 
	$(ARM) $(LIBTOOL) -static -o $(PRODUCTFOLDER)/$@/lib/libvc.a $(PRODUCTFOLDER)/$@/lib/libvc_.a $(SSLLIB)/$@/lib/libcrypto.a $(SSLLIB)/$@/lib/libssl.a
	@rm $(PRODUCTFOLDER)/$@/lib/libvc_.a

$(TMPFOLDER)/darwin-arm64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(ARM) $(CC) -I$(SSLINCLUDE) $(LCFLAGS) -isysroot $(MACOS_SDK) -c -o $@ $< -target macos-arm64

ios-simulator-x86_64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/ios-simulator-x86_64/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(X86) $(AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc_.a $^
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc
	$(X86) $(LIBTOOL) -static -o $(PRODUCTFOLDER)/$@/lib/libvc.a $(PRODUCTFOLDER)/$@/lib/libvc_.a $(SSLLIB)/$@/lib/libcrypto.a $(SSLLIB)/$@/lib/libssl.a
	@rm $(PRODUCTFOLDER)/$@/lib/libvc_.a

$(TMPFOLDER)/ios-simulator-x86_64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(X86) $(CC) -I$(SSLINCLUDE) $(LCFLAGS) -isysroot $(IOS_SIM_SDK) -c -o $@ $< -target x86_64-apple-ios-simulator

ios-simulator-arm64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/ios-simulator-arm64/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(ARM) $(AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc_.a $^
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc
	$(ARM) $(LIBTOOL) -static -o $(PRODUCTFOLDER)/$@/lib/libvc.a $(PRODUCTFOLDER)/$@/lib/libvc_.a $(SSLLIB)/$@/lib/libcrypto.a $(SSLLIB)/$@/lib/libssl.a
	@rm $(PRODUCTFOLDER)/$@/lib/libvc_.a

$(TMPFOLDER)/ios-simulator-arm64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(ARM) $(CC) -I$(SSLINCLUDE) $(LCFLAGS) -isysroot $(IOS_SIM_SDK) -c -o $@ $< -target arm64-apple-ios-simulator

ios-arm64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/ios-arm64/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(ARM) $(AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc_.a $^
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc
	$(ARM) $(LIBTOOL) -static -o $(PRODUCTFOLDER)/$@/lib/libvc.a $(PRODUCTFOLDER)/$@/lib/libvc_.a $(SSLLIB)/$@/lib/libcrypto.a $(SSLLIB)/$@/lib/libssl.a
	@rm $(PRODUCTFOLDER)/$@/lib/libvc_.a

$(TMPFOLDER)/ios-arm64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(ARM) $(CC) -I$(SSLINCLUDE) $(LCFLAGS) -fembed-bitcode -isysroot $(IOS_SDK) -c -o $@ $< -target arm64-apple-ios

android-armv8: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/android-armv8/$(CFILE)))
	@mkdir -p $(ANDROIDFOLDER)/arm64-v8a
	$(X86) $(ANDROID_AR) rcs -v $(ANDROIDFOLDER)/arm64-v8a/libvc.a $^
	@cp $(SSLLIB)/$@/lib/libcrypto.a $(ANDROIDFOLDER)/arm64-v8a/libcrypto.a
	@cp $(SSLLIB)/$@/lib/libssl.a $(ANDROIDFOLDER)/arm64-v8a/libssl.a

$(TMPFOLDER)/android-armv8/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(X86) $(ANDROID_ARM64_CROSS_CC) -I$(SSLINCLUDE) $(LCFLAGS) -c -o $@ $<

android-armv7: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/android-armv7/$(CFILE)))
	@mkdir -p $(ANDROIDFOLDER)/armeabi-v7a
	$(X86) $(ANDROID_AR) rcs -v $(ANDROIDFOLDER)/armeabi-v7a/libvc.a $^
	@cp $(SSLLIB)/$@/lib/libcrypto.a $(ANDROIDFOLDER)/armeabi-v7a/libcrypto.a
	@cp $(SSLLIB)/$@/lib/libssl.a $(ANDROIDFOLDER)/armeabi-v7a/libssl.a

$(TMPFOLDER)/android-armv7/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(X86) $(ANDROID_ARMv7_CROSS_CC) -I$(SSLINCLUDE) $(LCFLAGS) -c -o $@ $<

android-x86: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/android-x86/$(CFILE)))
	@mkdir -p $(ANDROIDFOLDER)/x86
	$(X86) $(ANDROID_AR) rcs -v $(ANDROIDFOLDER)/x86/libvc.a $^
	@cp $(SSLLIB)/$@/lib/libcrypto.a $(ANDROIDFOLDER)/x86/libcrypto.a
	@cp $(SSLLIB)/$@/lib/libssl.a $(ANDROIDFOLDER)/x86/libssl.a

$(TMPFOLDER)/android-x86/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(X86) $(ANDROID_X86_CROSS_CC) -I$(SSLINCLUDE) $(LCFLAGS) -c -o $@ $<

android-x86_64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/android-x86_64/$(CFILE)))
	@mkdir -p $(ANDROIDFOLDER)/x86_64
	$(X86) $(ANDROID_AR) rcs -v $(ANDROIDFOLDER)/x86_64/libvc.a $^
	@cp $(SSLLIB)/$@/lib/libcrypto.a $(ANDROIDFOLDER)/x86_64/libcrypto.a
	@cp $(SSLLIB)/$@/lib/libssl.a $(ANDROIDFOLDER)/x86_64/libssl.a

$(TMPFOLDER)/android-x86_64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(X86) $(ANDROID_X86_64_CROSS_CC) -I$(SSLINCLUDE) $(LCFLAGS) -c -o $@ $<

docker-linux-arm64:
	docker build --tag linux-arm64 - < ./docker/Dockerfile-linux-arm64
	docker run -v $(shell pwd):/root/vexl -v $(shell pwd)/../openssl:/root/openssl --rm --name linux-arm64 --platform=linux/aarch64 linux-arm64
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$(@:docker-%=%)/include/vc
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$(@:docker-%=%)/include/vc

linux-arm64: build-openssl-linux-arm64 build-linux-arm64

build-linux-arm64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/linux-arm64/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$(@:build-%=%)/lib $(PRODUCTFOLDER)/$(@:build-%=%)/include/vc
	$(CC) -shared \
		-Wl,--whole-archive $(SSLLIB)/$(@:build-%=%)/lib/libcrypto.a \
		-Wl,--whole-archive $(SSLLIB)/$(@:build-%=%)/lib/libssl.a \
		-o $(PRODUCTFOLDER)/$(@:build-%=%)/lib/libvc.so \
		-Wl,--no-whole-archive $^

$(TMPFOLDER)/linux-arm64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(SSLINCLUDE) $(LCFLAGS) -c -o $@ $<

docker-linux-x86_64:
	docker build --tag linux-x86_64 - < ./docker/Dockerfile-linux-x86_64
	docker run -v $(shell pwd):/root/vexl -v $(shell pwd)/../openssl:/root/openssl --rm --name linux-x86_64 --platform=linux/amd64 linux-x86_64
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$(@:docker-%=%)/include/vc
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$(@:docker-%=%)/include/vc

linux-x86_64: build-openssl-linux-x86_64 build-linux-x86_64

build-linux-x86_64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/linux-x86_64/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$(@:build-%=%)/lib $(PRODUCTFOLDER)/$(@:build-%=%)/include/vc
	$(CC) -shared \
		-Wl,--whole-archive $(SSLLIB)/$(@:build-%=%)/lib64/libcrypto.a \
		-Wl,--whole-archive $(SSLLIB)/$(@:build-%=%)/lib64/libssl.a \
		-o $(PRODUCTFOLDER)/$(@:build-%=%)/lib/libvc.so \
		-Wl,--no-whole-archive $^

$(TMPFOLDER)/linux-x86_64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(SSLINCLUDE) $(LCFLAGS) -c -o $@ $<

windows-x86_64: build-openssl-windows-x86_64 build-windows-x86_64

build-windows-x86_64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/windows-x86_64/$(CFILE)))
	$(eval ARCH := $(@:build-%=%))
	@mkdir -p $(PRODUCTFOLDER)/$(ARCH)/lib $(PRODUCTFOLDER)/$(ARCH)/include/vc
	x86_64-w64-mingw32-gcc -static-libgcc -shared -m64 \
		$^ \
		-o $(PRODUCTFOLDER)/$(ARCH)/lib/libvc.dll \
		-L$(SSLLIB)/$(ARCH)/lib64 -lssl -lcrypto -lws2_32 -lcrypt32
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$(ARCH)/include/vc
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$(ARCH)/include/vc

$(TMPFOLDER)/windows-x86_64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(WINDOWS_CROSS_CC) -I../openssl/lib/windows-x86_64/include -DBUILD_FOR_LIBRARY -fPIC -w -c -o $@ $<

test: $(CURRENTARCH) test-$(CURRENTARCH)

test-$(CURRENTARCH): $(TESTOFILES)
	@mkdir -p $(dir $(TESTBIN))
	$(CC) -lvc -L$(PRODUCTFOLDER)/$(CURRENTARCH)/lib -o $(TESTBIN) $^

$(TMPFOLDER)/$(CURRENTARCH)/$(TESTFOLDER)/%.o: $(TESTFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(PRODUCTFOLDER)/$(CURRENTARCH)/include $($(CFLAGS)) -c -o $@ $<

run:
	@$(TESTBIN)

run-performance:
	@$(TESTBIN) -p 100

clean:
	rm -rf $(DEPFILES) $(TMPFOLDER) $(PRODUCTFOLDER)
