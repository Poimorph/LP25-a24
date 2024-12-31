#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stddef.h>
#include <fcntl.h>
#define PORT 8080
int main(int argc, char const* argv[])
{
    int server_fd, new_socket;
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char* hello = "Hello from server";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket
         = accept(server_fd, (struct sockaddr*)&address,
                  &addrlen))
        < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    //valread = read(new_socket, buffer,
                   //1024 - 1); // subtract 1 for the null
                              // terminator at the end
    //printf("%s\n", buffer);

    // On recoit le chemin menant normalement au backup_log.txt

    char backup_log_path[1024 + 17];
    read(new_socket, backup_log_path, strlen(backup_log_path));
    printf("%s", backup_log_path);

    char * intbuf = "-1"; // On regarde si le fichier backup_log.txt existe, si oui on envoie une valeur différente de -1
    send(new_socket, intbuf, strlen(intbuf), 0);
    printf("Hello message sent\n");

    int value; // On recçois le nombre de fichier à télécharger depuis le client;
    read(new_socket, &value, sizeof(value));
    printf("Nombre d'éléments : %d\n", value);

    // Pour une sauvegarde incrémentale
    if (atoi(intbuf) == -1) {


        for (int i = 0; i < value; i++) {

            int type; // On reçoit le type du fichier, si celui ci est un fichier ou un dossier
            // O pour un fichier et 1 pour un dossier
            read(new_socket, &type, sizeof(type));

            char name[1024]; // On reçoit le nom du fichier / dossier
            read(new_socket, name, strlen(name));
            printf("%s", name);
            // dans le cas d'un dossier il faut juste le mkdir(name)
            // Et dans le cas d'un fichier il faut le créer et le remplir avec le code ci dessous
            // Il ne faut pas calculer la longueur dans le cas d'un dossier !!
            long size_of_file;
            read(new_socket, &size_of_file, sizeof(size_of_file));
            printf("Size : %d", size_of_file);
            FILE * file = fopen(name, "wb");
            char * buffer = malloc(sizeof(char) * size_of_file);

            read(new_socket, buffer, size_of_file);
            printf("%s", buffer);
            fwrite(buffer, sizeof(char), size_of_file, file);
            fclose(file);
            printf("len : %ld", strlen(buffer));



        }

        // On recoit le backup_log.txt :
        // Il ya juste à recopier ce qu'il y a dans la boucle pr le fichier et c'est bon

    }

    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    close(server_fd);
    return 0;
}
