FLAGS=-ggdb -Wall
CC=gcc
PROG=floodit

all: $(PROG)
debug: $(PROG)-debug

$(PROG): utils.o queue.o tree.o floodit.o main.o
	$(CC) $(FLAGS) utils.o queue.o tree.o floodit.o main.o -o $(PROG)

$(PROG)-debug: utils-debug.o queue.o tree.o floodit-debug.o main-debug.o
	$(CC) $(FLAGS) -DDEBUG utils-debug.o queue.o tree.o floodit-debug.o main-debug.o -o $(PROG)

main.o: main.c
	$(CC) -c $(FLAGS) main.c

main-debug.o: main.c
	$(CC) -c $(FLAGS) -DDEBUG main.c -o main-debug.o

floodit.o: floodit.c floodit.h
	$(CC) -c $(FLAGS) floodit.c

floodit-debug.o: floodit.c floodit.h
	$(CC) -c $(FLAGS) -DDEBUG floodit.c -o floodit-debug.o

tree.o: tree.c tree.h
	$(CC) -c $(FLAGS) tree.c

queue.o: queue.c queue.h
	$(CC) -c $(FLAGS) queue.c

utils.o: utils.c utils.h
	$(CC) -c $(FLAGS) utils.c

utils-debug.o: utils.c utils.h
	$(CC) -c $(FLAGS) -DDEBUG utils.c -o utils-debug.o

clean:
	rm -f *.o $(PROG)
