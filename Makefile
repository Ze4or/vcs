CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET = vcs
OBJ = main.o commands.o utils.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

main.o: main.c vcs.h
	$(CC) $(CFLAGS) -c main.c

commands.o: commands.c vcs.h utils.h
	$(CC) $(CFLAGS) -c commands.c

utils.o: utils.c utils.h vcs.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f *.o $(TARGET)
	rm -rf .vcs