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
    /* @param: data     - données d'entrée
     *         len      - taille des données
     *         md5_out  - buffer de sortie (doit être au moins MD5_DIGEST_LENGTH (=16) octets)
     */

    // Si les données ou la sortie sont nulles, on sort de la fonction
    if (data == NULL || md5_out == NULL) {
        return; 
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
    /* @param: hash_table   le tableau de hachage qui contient les MD5 et l'index des chunks unique
     *         md5          le md5 du chunk dont on veut déterminer l'unicité
     * 
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

void add_md5(Md5Entry *hash_table, unsigned char *md5, int index) {
    /* @brief Ajoute un MD5 dans la table de hachage
    *
    * @param   hash_table  Le tableau de hachage qui contiendra les entrées MD5
    *          md5         Le hash MD5 à ajouter dans la table de hachage
    *          index       L'index du chunk associé au hash MD5
    * 
    */
    // Calculer l'indice dans la table de hachage en utilisant la fonction de hachage
    unsigned int hash_index = hash_md5(md5);

    // Ajouter l'entrée dans la table à l'indice calculé
    // Copier le MD5 dans la table de hachage
    memcpy(hash_table[hash_index].md5, md5, MD5_DIGEST_LENGTH);

    // Enregistrer l'index du chunk dans la table
    hash_table[hash_index].index = index;
}

unsigned char *md5_file(FILE *file){
    if (!file) {
        fprintf(stderr, "Fichier invalide pour le calcul du MD5\n");
        return NULL;
    }
    
    // Allouer un buffer pour le résultat MD5
    unsigned char *md5_result = malloc(MD5_DIGEST_LENGTH);
    if (!md5_result) {
        fprintf(stderr, "Erreur : allocation mémoire pour le MD5\n");
        return NULL;
    }

    // Déterminer la taille du fichier
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Erreur : échec du déplacement à la fin du fichier\n");
        free(md5_result);
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size == -1) {
        fprintf(stderr, "Erreur : échec de la récupération de la taille du fichier\n");
        free(md5_result);
        return NULL;
    }

    rewind(file); // Revenir au début du fichier

    // Allouer un buffer pour lire le fichier
    unsigned char *file_buffer = malloc(file_size);
    if (!file_buffer) {
        fprintf(stderr, "Erreur : allocation mémoire pour le fichier\n");
        free(md5_result);
        return NULL;
    }

    // Lire le fichier entier
    size_t bytes_read = fread(file_buffer, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "Erreur : lecture du fichier incomplète\n");
        free(file_buffer);
        free(md5_result);
        return NULL;
    }

    // Calculer le MD5 du fichier
    compute_md5(file_buffer, file_size, md5_result);

    // Libérer le buffer temporaire
    free(file_buffer);

    return md5_result;

}

/**
 * @brief Fonction pour convertir un fichier non dédupliqué en tableau de chunks
 * 
 * @param file le fichier qui sera dédupliqué
 * @param chunks le tableau de chunks initialisés qui contiendra les chunks issu du fichier
 * @param hash_table le tableau de hachage qui contient les MD5 et l'index des chunks unique
 * , Chunk *chunks, Md5Entry *hash_table
 */
void deduplicate_file(FILE *file){
    // on détermine si le fichier est valide
    if (!file) {
        fprintf(stderr, "Erreur : Fichier invalide\n");
        return NULL;}

    // Déterminer la taille du fichier
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Erreur : échec du déplacement à la fin du fichier\n");
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size == -1) {
        fprintf(stderr, "Erreur : échec de la récupération de la taille du fichier\n");
        return NULL;
    }

    rewind(file); // Revenir au début du fichier
    printf("%lu\n",file_size);

}


int main() {

    FILE *file = fopen("src/example.txt", "rb");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return 1;
    }

    unsigned char *md5 = md5_file(file);
    fclose(file); 

    if (md5) {
        printf("MD5 du fichier : ");
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            printf("%02x", md5[i]);
        }
        printf("\n");
        free(md5); 
    }

    return 0;
}