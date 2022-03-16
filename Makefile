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

CC=gcc
AR=ar

TMPFOLDER=tmp
PRODUCTFOLDER=product
SRCFOLDER=src
TESTFOLDER=tests
OPENSSLFOLDER=openssl
SSLINCLUDE=$(OPENSSLFOLDER)/include
SSLLIB=$(OPENSSLFOLDER)/lib
TESTBIN=$(PRODUCTFOLDER)/$(TESTFOLDER)/test
ARCHITECTURES=darwin-x86_64 darwin-arm64

# Compiler flags
CFLAGS=-MP -MD -g -w

# ARCH variables
ARCHOFOLDERS=$(foreach ARCH,$(ARCHITECTURES),$(TMPFOLDER)/$(ARCH))
CURRENTARCH=$(shell uname | tr A-Z a-z)-$(shell uname -m | tr A-Z a-z)

# OpenSSL builds
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

all: $(OPENSSLTARGETS) $(ARCHITECTURES) test

$(OPENSSLTARGETS):
	./build.sh --$(@:build-openssl-%=%)


darwin-x86_64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/darwin-x86_64/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc.a $^ $(SSLLIB)/$@/lib/libcrypto.a $(SSLLIB)/$@/lib/libssl.a
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc 
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc 

$(TMPFOLDER)/darwin-x86_64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(SSLINCLUDE) $(CFLAGS) -c -DBUILD_FOR_LIBRARY -o $@ $< 



darwin-arm64: $(foreach CFILE, $(CFILES), $(patsubst %.c,%.o,$(TMPFOLDER)/darwin-arm64/$(CFILE)))
	@mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc.a $^ $(SSLLIB)/$@/lib/libcrypto.a $(SSLLIB)/$@/lib/libssl.a
	@cd $(SRCFOLDER) && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc 
	@cd $(SRCFOLDER) && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc 

$(TMPFOLDER)/darwin-arm64/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(SSLINCLUDE) $(CFLAGS) -c -DBUILD_FOR_LIBRARY -o $@ $< 

test: $(CURRENTARCH) test-$(CURRENTARCH)

test-$(CURRENTARCH): $(TESTOFILES)
	@mkdir -p $(dir $(TESTBIN))
	$(CC) -lvc -L$(PRODUCTFOLDER)/$(CURRENTARCH)/lib -I$(SSLINCLUDE) -o $(TESTBIN) $^ $(SSLLIB)/$(CURRENTARCH)/lib/libcrypto.a $(SSLLIB)/$(CURRENTARCH)/lib/libssl.a

$(TMPFOLDER)/$(CURRENTARCH)/$(TESTFOLDER)/%.o: $(TESTFOLDER)/%.c
	@mkdir -p $(dir $@)
	$(CC) -I$(PRODUCTFOLDER)/$(CURRENTARCH)/include $($(CFLAGS)) -c -o $@ $<

run:
	@$(TESTBIN)

clean:
	rm -rf $(DEPFILES) $(TMPFOLDER) $(PRODUCTFOLDER)

debug:
	$(eval TARGET := 'build-openssl-sdafas')
	@echo $(TARGET:build-openssl-%=%)