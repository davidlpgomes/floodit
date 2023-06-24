CC = gcc

CFLAGS =-ggdb -Wall
LFLAGS = -lm

PROG = floodit

all: $(PROG)
debug: $(PROG)-debug

$(PROG): utils.o floodit.o search.o main.o
	$(CC) $(CFLAGS) utils.o floodit.o search.o main.o -o $(PROG) $(LFLAGS)

$(PROG)-debug: utils-debug.o floodit-debug.o search-debug.o main-debug.o
	$(CC) $(CFLAGS) -DDEBUG utils-debug.o floodit-debug.o search-debug.o main-debug.o -o $(PROG) $(LFLAGS)

main.o: main.c
	$(CC) -c $(CFLAGS) main.c $(LFLAGS)

main-debug.o: main.c
	$(CC) -c $(CFLAGS) -DDEBUG main.c -o main-debug.o $(LFLAGS)

search.o: search.c search.h
	$(CC) -c $(CFLAGS) search.c $(LFLAGS)

search-debug.o: search.c search.h
	$(CC) -c $(CFLAGS) -DDEBUG search.c -o search-debug.o $(LFLAGS)

floodit.o: floodit.c floodit.h
	$(CC) -c $(CFLAGS) floodit.c $(LFLAGS)

floodit-debug.o: floodit.c floodit.h
	$(CC) -c $(CFLAGS) -DDEBUG floodit.c -o floodit-debug.o $(LFLAGS)

utils.o: utils.c utils.h
	$(CC) -c $(CFLAGS) utils.c $(LFLAGS)

utils-debug.o: utils.c utils.h
	$(CC) -c $(CFLAGS) -DDEBUG utils.c -o utils-debug.o $(LFLAGS)

clean:
	rm -f *.o $(PROG)
