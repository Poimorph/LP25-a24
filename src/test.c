#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <dirent.h>

#define CHUNK_SIZE 4096

// Taille de la table de hachage qui contiendra les chunks
// dont on a déjà calculé le MD5 pour effectuer les comparaisons
#define HASH_TABLE_SIZE 1000

// Structure pour un chunk
typedef struct {
    unsigned char md5[MD5_DIGEST_LENGTH]; // MD5 du chunk
    void *data; // Données du chunk
} Chunk;

// Table de hachage pour stocker les MD5 et leurs index
typedef struct {
    unsigned char md5[MD5_DIGEST_LENGTH];
    int index;
} Md5Entry;


// Fonction de hachage MD5 pour l'indexation
// dans la table de hachage
unsigned int hash_md5(unsigned char *md5) {
    unsigned int hash = 0;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        hash = (hash << 5) + hash + md5[i];
    }
    return hash % HASH_TABLE_SIZE;
    
}

// Fonction pour calculer le MD5 d'un chunk en utilisant OpenSSL EVP
void compute_md5(void *data, size_t len, unsigned char *md5_out) {
    /* @param: data - données d'entrée
     *         len - taille des données
     *         md5_out - buffer de sortie (doit être au moins MD5_DIGEST_LENGTH octets)
     */

    if (data == NULL || md5_out == NULL) {
        return; // Si les données ou la sortie sont nulles, on sort de la fonction
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Erreur : échec de l'initialisation du contexte EVP.\n");
        return;
    }

    // Initialisation pour MD5
    if (EVP_DigestInit_ex(ctx, EVP_md5(), NULL) != 1) {
        fprintf(stderr, "Erreur : échec de l'initialisation MD5.\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    // Mise à jour avec les données
    if (EVP_DigestUpdate(ctx, data, len) != 1) {
        fprintf(stderr, "Erreur : échec de la mise à jour MD5.\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    // Finalisation et obtention du résultat
    if (EVP_DigestFinal_ex(ctx, md5_out, NULL) != 1) {
        fprintf(stderr, "Erreur : échec de la finalisation MD5.\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    // Libération du contexte
    EVP_MD_CTX_free(ctx);
    // md5_out contient maintenant le hachage MD5 des données
}

// Fonction permettant de chercher un MD5 dans la table de hachage
int find_md5(Md5Entry *hash_table, unsigned char *md5) {
    /* @param: hash_table   est le tableau de hachage qui contient les MD5 et l'index des chunks unique
     *         md5          est le md5 du chunk dont on veut déterminer l'unicité
     * @return: retourne l'index s'il trouve le md5 dans le tableau et -1 sinon
     */

   // Calculer l'indice dans la table de hachage en utilisant la fonction de hachage
    unsigned int index = hash_md5(md5);

    // Vérifier la table à cet indice
    // Si l'entrée à cet indice correspond au MD5 cherché, retourner l'index
    if (memcmp(hash_table[index].md5, md5, MD5_DIGEST_LENGTH) == 0) {
        return hash_table[index].index;  // Retourner l'index du chunk correspondant
    }

    // Si aucune correspondance, retourner -1 pour indiquer que le MD5 n'a pas été trouvé
    return -1;
}

// Ajouter un MD5 dans la table de hachage
void add_md5(Md5Entry *hash_table, unsigned char *md5, int index) {
    // Calculer l'indice dans la table de hachage en utilisant la fonction de hachage
    unsigned int hash_index = hash_md5(md5);

    // Ajouter l'entrée dans la table à l'indice calculé
    // Copier le MD5 dans la table de hachage
    memcpy(hash_table[hash_index].md5, md5, MD5_DIGEST_LENGTH);

    // Enregistrer l'index du chunk dans la table
    hash_table[hash_index].index = index;
}

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