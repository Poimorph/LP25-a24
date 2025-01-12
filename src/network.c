#include "network.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

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
    if ((status = connect(client, (struct sockaddr *)&serv_addr,
                          sizeof(serv_addr))) < 0) {
        perror("Connection failed");
        return -1;
    }
    return client;
}

void send_data(int client, const void *data, size_t size) {
    // Implémenter la logique d'envoi de données à un serveur distant
    if (client <= 0) {
        fprintf(stderr, "Erreur : descripteur de socket client invalide\n");
        return;
    }

    if (!data || size == 0) {
        fprintf(stderr, "Erreur : données invalides à envoyer\n");
        return;
    }

    ssize_t sent = send(client, data, size, 0);
    if (sent < 0)
        perror("Erreur lors de l'envoi des données");
}

void receive_data(int client, void **data, size_t *size) {
    // Implémenter la logique de réception de données depuis un serveur distant

    if (client <= 0) {
        fprintf(stderr, "Erreur : descripteur de socket client invalide\n");
        return;
    }

    if (!data || !size) {
        fprintf(stderr, "Erreur : pointeurs de sortie invalides\n");
        return;
    }

    char *buffer = malloc(*size + 1);
    if (!buffer) {
        perror("Erreur d'allocation mémoire");
        return;
    }
    ssize_t received = read(client, buffer, *size); // subtract 1 for the null
                                                    // terminator at the end
    if (received < 0) {
        perror("Erreur lors de la réception");
        free(buffer);
        return;
    }

    buffer[received] = '\0';
    *data = buffer;
    *size = received;
}
