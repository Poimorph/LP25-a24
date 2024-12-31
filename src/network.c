#include "network.h"
#include <arpa/inet.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>


int start_connection(const char *server_address, int port) {
    if (!server_address || !port) {
        perror("Invalid port or server adress");
        return -1;
    }
    int client;
    if ((client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error during creation of socket");
        return -1;
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_address, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }
    int status;
    if ((status = connect(client, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        perror("Connection failed");
        return -1;
    }
    return client;

}

void send_data(int client, const void *data, size_t size) {
    // Implémenter la logique d'envoi de données à un serveur distant
    if (!client) {
        perror("Client invalide\n");
        return;
    }
    if (!data || !size) {
        perror("Données invalides");
        return;
    }
    send(client, data, size, 0);

}

void receive_data(int client, void **data, size_t *size) {
    // Implémenter la logique de réception de données depuis un serveur distant
    int value;
    char buffer[1024];
    char * hello = "Hello from client";
    send(client, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    value = read(client, buffer,
                   1024-1); // subtract 1 for the null
                              // terminator at the end

    printf("char : %d", atoi(buffer));

    *data = buffer;
}
