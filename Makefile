CC=gcc
AR=ar

TMPFOLDER=tmp
PRODUCTFOLDER=product
SRCFOLDER=src
TESTFOLDER=tests
SSLINCLUDE=openssl/include
TESTBIN=$(PRODUCTFOLDER)/$(TESTFOLDER)/test
ARCHITECTURES=darwin

# ARCH variables
ARCHOFOLDERS=$(foreach ARCH,$(ARCHITECTURES),$(TMPFOLDER)/$(ARCH))

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
$(foreach ARCHFOLDER,$(ARCHOFOLDERS), \
	$(foreach CFILE, $(TESTCFILES), \
		$(patsubst %.c,%.o,$(ARCHFOLDER)/$(CFILE)) \
	) \
)

# clean variables
ALLCFILES= $(CFILES) $(TESTCFILES)
DEPFILES=$(patsubst %.c,%.d,$(ALLCFILES))
-include $(DEPFILES)


all: $(ARCHITECTURES) test

darwin: $(OFILES)
	
	mkdir -p $(PRODUCTFOLDER)/$@/lib $(PRODUCTFOLDER)/$@/include/vc
	$(AR) rcs -v $(PRODUCTFOLDER)/$@/lib/libvc.a $^
	
	cd src && rsync -R ./**/*.h ../$(PRODUCTFOLDER)/$@/include/vc 
	cd src && rsync -R ./*.h ../$(PRODUCTFOLDER)/$@/include/vc 

$(TMPFOLDER)/darwin/$(SRCFOLDER)/%.o: $(SRCFOLDER)/%.c
	mkdir -p $(dir $@)
	./build.sh --darwin
	$(CC) -I$(SSLINCLUDE) -MP -MD -c -DBUILD_FOR_LIBRARY -o $@ $< -w

test: darwin test-darwin

test-darwin: $(TESTOFILES)
	mkdir -p $(dir $(TESTBIN))
	$(CC) -lvc -Lproduct/darwin/lib -I$(SSLINCLUDE) -o $(TESTBIN) $^ product/darwin/lib/libvc.a openssl/lib/darwin/lib/libcrypto.a openssl/lib/darwin/lib/libssl.a

$(TMPFOLDER)/darwin/$(TESTFOLDER)/%.o: $(TESTFOLDER)/%.c
	mkdir -p $(dir $@)
	$(CC) -Iproduct/darwin/include -MP -MD -c -o $@ $<

run:
	$(TESTBIN)

clean:
	rm -rf $(DEPFILES) $(TMPFOLDER) $(PRODUCTFOLDER)
