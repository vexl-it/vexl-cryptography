CC=gcc
AR=ar

ARCHITECTURES=darwin

SSLINCLUDE=openssl/include

SRCFOLDER=src
CFILES=$(foreach D,$(SRCFOLDER),$(wildcard $(D)/*.c))
OFILES=$(patsubst %.c,%.o,$(CFILES))

TESTFOLDER=tests
TESTCFILES=$(foreach D,$(TESTFOLDER),$(wildcard $(D)/*.c))
TESTOFILES=$(patsubst %.c,%.o,$(TESTCFILES))
TESTBIN=product/tests/test

ALLCFILES= $(CFILES) $(TESTCFILES)
DEPFILES=$(patsubst %.c,%.d,$(ALLCFILES))
-include $(DEPFILES)


all: $(ARCHITECTURES) test

darwin: tmp/darwin/$(OFILES)
	rm -Rf product/$@
	mkdir -p product/$@/lib product/$@/include/vc
	$(AR) rcs -v product/$@/lib/libvc.a $^
	cp src/*.h product/$@/include/vc/

tmp/darwin/src/%.o: $(CFILES)
	mkdir -p $(dir $@)
	./build.sh --darwin
	$(CC) -g -I$(SSLINCLUDE) -MP -MD -c -DBUILD_FOR_LIBRARY -o $@ $<

test: darwin test-darwin

test-darwin: tmp/darwin/$(TESTOFILES)
	mkdir -p $(dir $(TESTBIN))
	$(CC) -lvc -Lproduct/darwin/lib -g -I$(SSLINCLUDE) -o $(TESTBIN) $< product/darwin/lib/libvc.a openssl/lib/darwin/lib/libcrypto.a openssl/lib/darwin/lib/libssl.a

tmp/darwin/tests/%.o: $(TESTCFILES)
	mkdir -p $(dir $@)
	$(CC) -Iproduct/darwin/include -MP -MD -c -o $@ $<

run:
	$(TESTBIN)

clean:
	rm -rf $(TESTBIN) $(OFILES) $(TESTOFILES) $(DEPFILES) tmp product
