#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "file_handler.h"
#include "deduplication.h"
#include "backup_manager.h"
#include "network.h"

//void testMD5Compute(){
//    // Exemple de données pour lesquelles on va calculer le MD5
//    const char *data = "Hello, world!";
//    ssize_t len = strlen(data);  // Calculer la longueur des données
//
//    // Tableau pour stocker le résultat du hash MD5 (16 octets)
//    unsigned char md5_hash[MD5_DIGEST_LENGTH];
//
//    // Calculer le MD5 des données
//    compute_md5((void *)data, len, md5_hash);
//
//    // Afficher le hash MD5 au format hexadécimal
//    printf("Hash MD5 : ");
//    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
//        printf("%02x", md5_hash[i]);  // Afficher chaque octet en hexadécimal
//    }
//    printf("\n");
//}
//
//void testAddEtFindMD5(){
//    // Allocation et initialisation de la table de hachage
//    Md5Entry hash_table[HASH_TABLE_SIZE] = {{0}};
//
//    // Exemple de chunk
////    unsigned char md5_sample[MD5_DIGEST_LENGTH] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF};
//
//    // Ajouter un MD5 à la table
////    add_md5(hash_table, md5_sample, 1);
//
//    // Chercher le MD5 dans la table
////    int index = find_md5(hash_table, md5_sample);
//
//    if (index != -1) {
//        printf("MD5 trouvé, index : %d\n", index);
//    } else {
//        printf("MD5 non trouvé\n");
//    }
//}


int main(int argc, char *argv[]) {
    // tests :
    // testMD5Compute();

    // Analyse des arguments de la ligne de commande

    // Implémentation de la logique de sauvegarde et restauration
    // Exemples : gestion des options --backup, --restore, etc.
    
    return EXIT_SUCCESS;
}

