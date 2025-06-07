CC = $(shell xcrun --sdk macosx --find clang)
SDK = $(shell xcrun --sdk macosx --show-sdk-path)
LDID = $(shell which ldid)
LIBS = $(SDK)/usr/lib

CFLAGS := -I./include -isysroot $(SDK) -arch x86_64
LDFLAGS += -F$(SDK)/System/Library/PrivateFrameworks -F/Library/Apple/System/Library/PrivateFrameworks
LDFLAGS +=  -L$(SDK)/usr/lib -lSystem -lc -framework CoreFoundation -framework MobileDevice
LDFLAGS += -Wl,-sectcreate,__DATA,__payload_dmg,../../resources/payload.dmg
LDFLAGS += -Wl,-sectcreate,__DATA,__stock_dmg,../../resources/stock.dmg
LDFLAGS += -Wl,-sectcreate,__DATA,__signature,../../resources/stock.signature
LDFLAGS += -Wl,-sectcreate,__DATA,__bootstrap,../../resources/bootstrap.tar
LDFLAGS += -Wl,-sectcreate,__DATA,__launchd_conf,../../resources/launchd.conf
LDFLAGS += -Wl,-sectcreate,__DATA,__splashscreen,../../resources/splashscreen.jp2
LDFLAGS += -Wl,-sectcreate,__DATA,__libmis,../../resources/_libmis.dylib
LDFLAGS += -Wl,-sectcreate,__DATA,__tar,../../resources/tar
LDFLAGS += -Wl,-sectcreate,__DATA,__aquila,../../untether/aquila
LDFLAGS += -Wl,-sectcreate,__DATA,__installer,../../installer/installer

SRC = $(wildcard src/*.c)

all: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o aquila $^
	$(LDID) -S aquila

clean:
	@rm -rf aquila
	@rm -rf *.o
