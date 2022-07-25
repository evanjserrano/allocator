
CC=clang
CFLAGS=-g -Wall -Wno-deprecated-declarations
OBJS=alloc.o main.o
BIN=alloc

all: CFLAGS += -g3 -O3
all: executable

debug: CFLAGS += -DDEBUG -DCLEAN_MEMORY
debug: executable

executable: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(BIN) *.o

run: all
	./$(BIN)