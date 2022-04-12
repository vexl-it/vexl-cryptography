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

MACOS_SDK=$(shell xcrun --sdk macosx --show-sdk-path)
IOS_SIM_SDK=$(shell xcrun --sdk iphonesimulator --show-sdk-path)
IOS_SDK=$(shell xcrun --sdk iphoneos --show-sdk-path)

TMPFOLDER=tmp
PRODUCTFOLDER=product
SRCFOLDER=src
TESTFOLDER=tests
OPENSSLFOLDER=openssl
SSLINCLUDE=$(OPENSSLFOLDER)/include
SSLLIB=$(OPENSSLFOLDER)/lib
TESTBIN=$(PRODUCTFOLDER)/$(TESTFOLDER)/test
APPLE_ARCHITECTURES=darwin-x86_64 darwin-arm64 ios-simulator-x86_64 ios-simulator-arm64 ios-arm64
ARCHITECTURES=$(APPLE_ARCHITECTURES)

# Compiler flags
CFLAGS=-MP -MD -g -w -O3
LCFLAGS= $(CFLAGS) -DBUILD_FOR_LIBRARY

# ARCH variables
ARCHOFOLDERS=$(foreach ARCH,$(ARCHITECTURES),$(TMPFOLDER)/$(ARCH))
CURRENTARCH=$(shell uname | tr A-Z a-z)-$(shell uname -m | tr A-Z a-z)

# OpenSSL builds
OPENSSL_APPLE_TARGETS=$(foreach ARCH, $(APPLE_ARCHITECTURES), build-openssl-$(ARCH))
OPENSSLTARGETS=$(foreach ARCH, $(ARCHITECTURES), build-openssl-$(ARCH))

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

all: apple test

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

test: $(CURRENTARCH) test-$(CURRENTARCH)

test-$(CURRENTARCH): $(TESTOFILES)
	@mkdir -p $(dir $(TESTBIN))
	$(CC) -lvc -L$(PRODUCTFOLDER)/$(CURRENTARCH)/lib -o $(TESTBIN) $^

$(TMPFOLDER)/$(CURRENTARCH)/$(TESTFOLDER)/%.o: $(TESTFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(PRODUCTFOLDER)/$(CURRENTARCH)/include $($(CFLAGS)) -c -o $@ $<

run:
	@$(TESTBIN)

run-test:
	@$(TESTBIN) -p 1000

clean:
	rm -rf $(DEPFILES) $(TMPFOLDER) $(PRODUCTFOLDER)
