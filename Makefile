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

clean:
	rm -f $(BIN)
