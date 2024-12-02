#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "file_handler.h"
#include "deduplication.h"
#include "backup_manager.h"
#include "network.h"

void testMD5Compute(){
    // Exemple de données pour lesquelles on va calculer le MD5
    const char *data = "Hello, world!";
    size_t len = strlen(data);  // Calculer la longueur des données
    
    // Tableau pour stocker le résultat du hash MD5 (16 octets)
    unsigned char md5_hash[MD5_DIGEST_LENGTH];

    // Calculer le MD5 des données
    compute_md5((void *)data, len, md5_hash);

    // Afficher le hash MD5 au format hexadécimal
    printf("Hash MD5 : ");
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", md5_hash[i]);  // Afficher chaque octet en hexadécimal
    }
    printf("\n");
}


int main(int argc, char *argv[]) {
    // tests :
    testMD5Compute();

    // Analyse des arguments de la ligne de commande

    // Implémentation de la logique de sauvegarde et restauration
    // Exemples : gestion des options --backup, --restore, etc.
    
    return EXIT_SUCCESS;
}

