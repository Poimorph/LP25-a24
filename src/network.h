#ifndef NETWORK_H
#define NETWORK_H
#include <stddef.h>

int start_connection(const char *server_address, int port);
void send_data(int client, const void *data, size_t size);
void receive_data(int client, void **data, size_t *size);

#endif // NETWORK_H
