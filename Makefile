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
SRCFOLDER=src
TESTFOLDER=tests
OPENSSLFOLDER=../openssl
SSLINCLUDE=$(OPENSSLFOLDER)/include
SSLLIB=$(OPENSSLFOLDER)/lib
TESTBIN=$(PRODUCTFOLDER)/$(TESTFOLDER)/test

# Compiler flags
CFLAGS=-MP -MD -g -w
LCFLAGS= $(CFLAGS) -DBUILD_FOR_LIBRARY

# ARCH variables
APPLE_ARCHITECTURES=darwin-x86_64 darwin-arm64 ios-simulator-x86_64 ios-simulator-arm64 ios-arm64
ANDROID_ARCHITECTURES=android-armv8 android-armv4 android-x86 android-x86_64
DOCKER_LINUX_ARCHITECTURES=docker-linux-arm64 docker-linux-x86_64
LINUX_ARCHITECTURES=linux-arm64 linux-x86_64
ARCHITECTURES=$(APPLE_ARCHITECTURES) $(APPLE_ARCHITECTURES) $(DOCKER_LINUX_ARCHITECTURES)

ARCHOFOLDERS=$(foreach ARCH,$(ARCHITECTURES),$(TMPFOLDER)/$(ARCH))
CURRENTARCH=$(shell uname | tr A-Z a-z)-$(shell uname -m | tr A-Z a-z)

MACOS_SDK=$(shell xcrun --sdk macosx --show-sdk-path)
IOS_SIM_SDK=$(shell xcrun --sdk iphonesimulator --show-sdk-path)
IOS_SDK=$(shell xcrun --sdk iphoneos --show-sdk-path)

ANDROID_NDK_ROOT=~/Library/Android/sdk/ndk/24.0.8215888
ANDROID_ARM64_CROSS_CC=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/darwin-x86_64/bin/aarch64-linux-android32-clang
ANDROID_ARMv7_CROSS_CC=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/darwin-x86_64/bin/armv7a-linux-androideabi32-clang
ANDROID_X86_64_CROSS_CC=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/darwin-x86_64/bin/x86_64-linux-android32-clang
ANDROID_X86_CROSS_CC=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/darwin-x86_64/bin/i686-linux-android32-clang
ANDROID_AR=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/darwin-x86_64/bin/llvm-ar
# OpenSSL builds
OPENSSL_APPLE_TARGETS=$(foreach ARCH, $(APPLE_ARCHITECTURES), build-openssl-$(ARCH))
OPENSSL_ANDROID_TARGETS=$(foreach ARCH, $(ANDROID_ARCHITECTURES), build-openssl-$(ARCH))
OPENSSL_LINUX_TARGETS=$(foreach ARCH, $(LINUX_ARCHITECTURES), build-openssl-$(ARCH))
OPENSSLTARGETS=$(OPENSSL_APPLE_TARGETS) $(OPENSSL_ANDROID_TARGETS) $(OPENSSL_LINUX_TARGETS)

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

all: apple android linux test

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
	xcodebuild -create-xcframework $(LIBS) -output $(PRODUCTFOLDER)/apple/vexl.xcframework
	zip -q -r $(PRODUCTFOLDER)/apple/vexl_crypto_ios_xcframework.zip $(PRODUCTFOLDER)/apple/vexl.xcframework

android: $(OPENSSL_ANDROID_TARGETS) $(ANDROID_ARCHITECTURES)
	@mkdir -p $(PRODUCTFOLDER)/android
	cd $(PRODUCTFOLDER) && zip -r android/vexl_crypto_android_frameworks.zip android-armv8 android-armv4 android-x86 android-x86_64

linux: $(DOCKER_LINUX_ARCHITECTURES)
	@mkdir -p $(PRODUCTFOLDER)/linux
	cd $(PRODUCTFOLDER) && zip -r linux/vexl_crypto_linux_frameworks.zip linux-arm64 linux-x86_64

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
	$(ARM) $(CC) -I$(SSLINCLUDE) $(LCFLAGS) -isysroot $(IOS_SDK) -c -o $@ $< -target arm64-apple-ios

android-armv8: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/android-armv8/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(X86) $(ANDROID_AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc.a $^
	@cp $(SSLLIB)/$@/lib/libcrypto.a $(PRODUCTFOLDER)/$@/lib/libcrypto.a
	@cp $(SSLLIB)/$@/lib/libssl.a $(PRODUCTFOLDER)/$@/lib/libssl.a
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc

$(TMPFOLDER)/android-armv8/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(X86) $(ANDROID_ARM64_CROSS_CC) -I$(SSLINCLUDE) $(LCFLAGS) -c -o $@ $<

android-armv4: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/android-armv4/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(X86) $(ANDROID_AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc.a $^
	@cp $(SSLLIB)/$@/lib/libcrypto.a $(PRODUCTFOLDER)/$@/lib/libcrypto.a
	@cp $(SSLLIB)/$@/lib/libssl.a $(PRODUCTFOLDER)/$@/lib/libssl.a
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc

$(TMPFOLDER)/android-armv4/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(X86) $(ANDROID_ARMv7_CROSS_CC) -I$(SSLINCLUDE) $(LCFLAGS) -c -o $@ $<

android-x86: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/android-x86/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(X86) $(ANDROID_AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc.a $^
	@cp $(SSLLIB)/$@/lib/libcrypto.a $(PRODUCTFOLDER)/$@/lib/libcrypto.a
	@cp $(SSLLIB)/$@/lib/libssl.a $(PRODUCTFOLDER)/$@/lib/libssl.a
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc

$(TMPFOLDER)/android-x86/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(X86) $(ANDROID_X86_CROSS_CC) -I$(SSLINCLUDE) $(LCFLAGS) -c -o $@ $<


android-x86_64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/android-x86_64/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(X86) $(ANDROID_AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc.a $^
	@cp $(SSLLIB)/$@/lib/libcrypto.a $(PRODUCTFOLDER)/$@/lib/libcrypto.a
	@cp $(SSLLIB)/$@/lib/libssl.a $(PRODUCTFOLDER)/$@/lib/libssl.a
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc

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
	$(AR) rcs -v $(PRODUCTFOLDER)/$(@:build-%=%)/lib/libvc.a $^
	@cp $(SSLLIB)/$(@:build-%=%)/lib/libcrypto.a $(PRODUCTFOLDER)/$(@:build-%=%)/lib/libcrypto.a
	@cp $(SSLLIB)/$(@:build-%=%)/lib/libssl.a $(PRODUCTFOLDER)/$(@:build-%=%)/lib/libssl.a

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
	$(AR) rcs -v $(PRODUCTFOLDER)/$(@:build-%=%)/lib/libvc.a $^
	@cp $(SSLLIB)/$(@:build-%=%)/lib64/libcrypto.a $(PRODUCTFOLDER)/$(@:build-%=%)/lib/libcrypto.a
	@cp $(SSLLIB)/$(@:build-%=%)/lib64/libssl.a $(PRODUCTFOLDER)/$(@:build-%=%)/lib/libssl.a

$(TMPFOLDER)/linux-x86_64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(SSLINCLUDE) $(LCFLAGS) -c -o $@ $<

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
