#include <stdio.h>
#include <string.h>
#include "deduplication.h" 

int main() {
    // Données d'exemple
    const char *data1 = "Ceci est un chunk de données."; // Chunk 1
    const char *data2 = "Ceci est un autre chunk de données."; // Chunk 2
    const char *data3 = "Ceci est un chunk de données."; // Identique à Chunk 1

    // Calculer les MD5 des chunks
    unsigned char md5_1[MD5_DIGEST_LENGTH];
    unsigned char md5_2[MD5_DIGEST_LENGTH];
    unsigned char md5_3[MD5_DIGEST_LENGTH];

    compute_md5((void *)data1, strlen(data1), md5_1);
    compute_md5((void *)data2, strlen(data2), md5_2);
    compute_md5((void *)data3, strlen(data3), md5_3);

    // Initialiser une table de hachage
    Md5Entry hash_table[HASH_TABLE_SIZE] = {0};

    // Ajouter les MD5 à la table de hachage
    add_md5(hash_table, md5_1, 0); // Ajouter le chunk 1 avec l'index 0
    add_md5(hash_table, md5_2, 1); // Ajouter le chunk 2 avec l'index 1

    // Tester la recherche dans la table de hachage
    int index1 = find_md5(hash_table, md5_1); // Devrait trouver l'index 0
    int index2 = find_md5(hash_table, md5_2); // Devrait trouver l'index 1
    int index3 = find_md5(hash_table, md5_3); // Devrait aussi trouver l'index 0 (même données)

    // Afficher les résultats
    printf("Index du chunk 1 : %d\n", index1); // Doit afficher 0
    printf("Index du chunk 2 : %d\n", index2); // Doit afficher 1
    printf("Index du chunk 3 (identique au chunk 1) : %d\n", index3); // Doit afficher 0

    return 0;
}