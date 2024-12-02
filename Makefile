CC = gcc
CFLAGS = -Wall -Wextra -I./src
SRC = src/main.c src/file_handler.c src/deduplication.c src/backup_manager.c src/network.c
OBJ = $(SRC:.c=.o)

all: cborgbackup

cborgbackup: $(OBJ)
	$(CC) -o cborgbackup $(OBJ)

clean:
	rm -f $(OBJ) cborgbackup
