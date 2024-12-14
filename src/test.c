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
size_t deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table){
    // on détermine si le fichier est valide
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return NULL;}

    // Déterminer la taille du fichier
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Erreur lors du parcours du fichier");
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size == -1) {
        perror("Erreur lors de la récupération de la taille du fichier");
        return NULL;
    }

    rewind(file); // Revenir au début du fichier
    
    
    size_t chunk_count = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE; // Nombre de chunks nécessaires
    
    for (size_t i = 0; i < chunk_count; i++) {
        size_t bytes_to_read = CHUNK_SIZE;
        if (i == chunk_count - 1) { // Dernier chunk
            bytes_to_read = file_size % CHUNK_SIZE;
            if (bytes_to_read == 0) bytes_to_read = CHUNK_SIZE;
        }

        // Allouer un buffer pour le chunk
        unsigned char *buffer = malloc(bytes_to_read);
        if (!buffer) {
            perror("Erreur lors de l'allocation mémoire pour le chunk");
            return;
        }

        // Lire les données du chunk
        size_t bytes_read = fread(buffer, 1, bytes_to_read, file);
        if (bytes_read != bytes_to_read) {
            perror("Erreur lors de la lecture du chunk");
            free(buffer);
            return;
        }

        // Calculer le MD5 du chunk
        unsigned char md5[MD5_DIGEST_LENGTH];
        compute_md5(buffer, bytes_read, md5);

        // Vérifier si le MD5 existe déjà dans la table de hachage
        int existing_index = find_md5(hash_table, md5);
        if (existing_index != -1) {
            // Le chunk existe déjà, pas besoin de l'ajouter
            free(buffer);
            continue;
        }

        // Ajouter le MD5 dans la table de hachage
        add_md5(hash_table, md5, i);

        // Remplir la structure Chunk
        memcpy(chunks[i].md5, md5, MD5_DIGEST_LENGTH);
        chunks[i].data = buffer;

    }
    return chunk_count;
}

#define MAX_CHUNKS 1000 // Taille maximale du tableau de chunks

int main() {

     // Fichier de test
    const char *test_file = "src/example.txt";

    

    // Initialiser les structures nécessaires
    Chunk chunks[CHUNK_SIZE] = {0}; // Tableau pour stocker les chunks uniques
    Md5Entry hash_table[HASH_TABLE_SIZE] = {0}; // Table de hachage pour les MD5
    size_t nb;

    // Ouvrir le fichier pour la déduplication
    FILE *file = fopen(test_file, "rb");
    if (!file) {
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier de test\n");
        return EXIT_FAILURE;
    }

    // Appeler la fonction de déduplication
    nb=deduplicate_file(file, chunks, hash_table);
    fclose(file);

    // Afficher les résultats
    printf("Chunks uniques détectés :\n");
    for (int i = 0; i < CHUNK_SIZE && chunks[i].data != NULL; i++) {
        printf("Chunk %d : ", i + 1);
        for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
            printf("%02x", chunks[i].md5[j]); // Afficher le MD5

        }
        printf("\n");
    }
    printf("nombre total de chunks : %zu",nb);

    // Libérer la mémoire allouée pour les chunks
    for (int i = 0; i < CHUNK_SIZE && chunks[i].data != NULL; i++) {
        free(chunks[i].data);
    }

     


    return 0;
}