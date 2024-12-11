CC = gcc
CFLAGS = -Wall -Wextra -I./src -lcrypto
SRC = src/main.c src/file_handler.c src/deduplication.c src/backup_manager.c src/network.c
OBJ = $(SRC:.c=.o)

all: cborgbackup

%.o: %.c
	gcc -o $@ $<

main: main.o backup_manager.o
	gcc -o $@ $<

cborgbackup: $(OBJ)
	$(CC) -o cborgbackup $(OBJ)

clean:
	rm -f $(OBJ) cborgbackup
