CC = gcc
CFLAGS = -Wall -Wextra -I./src

# Flags des bibliothèques
LDFLAGS = -lcrypto -lssl

SRC = src/main.c src/file_handler.c src/deduplication.c src/backup_manager.c src/network.c src/options.c
OBJ = $(SRC:.c=.o)
TARGET = cborgbackup
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
