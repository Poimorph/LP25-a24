#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stdio.h>
#include <openssl/md5.h>


// Structure pour une ligne du fichier log
typedef struct log_element{
    const char *path; // Chemin du fichier/dossier
    unsigned char md5[MD5_DIGEST_LENGTH]; // MD5 du fichier dédupliqué
    char *date; // Date de dernière modification
    struct log_element *next;
    struct log_element *prev;
} log_element;

// Structure pour une liste de log représentant le contenu du fichier backup_log
typedef struct {
    log_element *head; // Début de la liste de log
    log_element *tail; // Fin de la liste de log
} log_t;

// Structure pour stocker une liste dynamique de chemins
typedef struct {
    char **paths;    // Tableau de chaînes représentant les chemins
    int count;       // Nombre actuel de chemins dans la liste
    int capacity;    // Capacité actuelle du tableau
} PathList;

int hex_to_int(unsigned char c);
void md5_hex_to_bytes(unsigned char * hex_md5, unsigned char * md5_bytes);
log_t *read_backup_log(const char *logfile);
void update_backup_log(const log_element *element, const char *filename);
void write_log_element(log_element *elt, const char *logfile);
PathList *list_files(const char *directory);
void copy_file(const char *src, const char *dest);

#endif // FILE_HANDLER_H