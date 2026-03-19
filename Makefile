CC     = cc
SRC    = src/main.c
BIN    = softrend
CFLAGS = -I include -O2

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
    LDFLAGS = -framework Cocoa
else ifeq ($(UNAME), Linux)
    LDFLAGS = -lX11
else
    LDFLAGS = -lgdi32
endif

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LDFLAGS)

asm:
	$(CC) -I include -O2 -g -S -fverbose-asm $(SRC) -o src/main.s

dump:
	$(CC) $(CFLAGS) -g $(SRC) -o $(BIN)_debug $(LDFLAGS)
	objdump --source $(BIN)_debug > src/main.dump

clean:
	rm -f $(BIN)
