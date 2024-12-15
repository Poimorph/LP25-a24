#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <dirent.h>



/**
 * @brief Fonction de hachage MD5 pour l'indexation dans la table de hachage
 * 
 * @param md5 Tableau de taille MD5_DIGEST_LENGTH
 * @return unsigned int - L'index calculé dans la plage [0, HASH_TABLE_SIZE - 1].
 */
unsigned int hash_md5(unsigned char *md5) {
    unsigned int hash = 0;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        hash = (hash << 5) + hash + md5[i];
    }
    return hash % HASH_TABLE_SIZE;
    
}

/**
 * @brief Calcule le hash MD5 des données données en entrée.
 * 
 * Cette fonction utilise l'API OpenSSL EVP pour calculer un hash MD5 des données 
 * spécifiées. Le résultat est écrit dans le buffer `md5_out`, qui doit avoir une 
 * taille de MD5_DIGEST_LENGTH (16 octets).
 * 
 * @param data     Un pointeur vers les données d'entrée dont le MD5 doit être calculé.
 *                 Ce pointeur ne doit pas être NULL.
 * @param len      La taille des données en octets.
 * @param md5_out  Un pointeur vers un buffer où le résultat du MD5 sera stocké. 
 *                 Ce buffer doit avoir une taille de MD5_DIGEST_LENGTH (16 octets). 
 *                 Ce pointeur ne doit pas être NULL.
 * 
 * @note Si `data` ou `md5_out` est NULL, la fonction retourne immédiatement sans effectuer de calcul.
 * 
 * @warning En cas d'échec de l'initialisation ou des étapes de calcul, un message d'erreur est écrit sur `stderr`.
 * 
 * @see EVP_MD_CTX_new, EVP_DigestInit_ex, EVP_DigestUpdate, EVP_DigestFinal_ex
 */
void compute_md5(void *data, size_t len, unsigned char *md5_out) {
    // Si les données ou la sortie sont nulles, on sort de la fonction
    if (data == NULL || md5_out == NULL) {
        return; 
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        perror("Erreur lors de l'initialisation du contexte EVP.\n");
        return;
    }

    // Initialisation pour MD5
    if (EVP_DigestInit_ex(ctx, EVP_md5(), NULL) != 1) {
        perror("Erreur lors de échec de l'initialisation MD5.\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    // Mise à jour avec les données
    if (EVP_DigestUpdate(ctx, data, len) != 1) {
        perror("Erreur lors de échec de la mise à jour MD5.\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    // Finalisation et obtention du résultat
    if (EVP_DigestFinal_ex(ctx, md5_out, NULL) != 1) {
        perror("Erreur lors de échec de la finalisation MD5.\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    // Libération du contexte
    EVP_MD_CTX_free(ctx);

    // md5_out contient maintenant le hachage MD5 des données
}

/**
 * @brief Fonction permettant de chercher un MD5 dans la table de hachage
 * 
 * @param hash_table le tableau de hachage qui contient les MD5 et l'index des chunks unique
 * @param md5 le md5 du chunk dont on veut déterminer l'unicité
 * 
 * @return retourne l'index s'il trouve le md5 dans le tableau et -1 sinon
 */
int find_md5(Md5Entry *hash_table, unsigned char *md5) {
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

/**
 * @brief Fonction pour ajouter un MD5 dans la table de hachage.
 * 
 * Cette fonction insère un hash MD5 et son index associé dans une table de hachage. 
 * Elle utilise une fonction de hachage pour déterminer l'emplacement où stocker les données. 
 * 
 * @param hash_table Le tableau de hachage qui contiendra les entrées MD5
 * @param md5 Le hash MD5 à ajouter dans la table de hachage
 * @param index L'index du chunk associé au hash MD5
 */
void add_md5(Md5Entry *hash_table, unsigned char *md5, int index) {
    // Calculer l'indice dans la table de hachage en utilisant la fonction de hachage
    unsigned int hash_index = hash_md5(md5);
 
    if(memcmp(hash_table[hash_index].md5, md5, MD5_DIGEST_LENGTH)!= 0){ //Verifie que le md5 n'a pas deja été enregistré
        // Ajouter l'entrée dans la table à l'indice calculé 
        // Copier le MD5 dans la table de hachage 
        memcpy(hash_table[hash_index].md5, md5, MD5_DIGEST_LENGTH);

        // Enregistrer l'index du chunk dans la table
        hash_table[hash_index].index = index;
        }
}

/**
 * @brief  Calcule le hash MD5 d'un fichier complet.
 *
 * @param  file Pointeur vers un fichier ouvert en mode binaire (FILE*), dont le MD5 sera calculé.
 * 
 * @return unsigned char* - Un tableau de 16 octets contenant le hash MD5 du fichier.
 *                          Renvoie NULL en cas d'erreur (fichier invalide, allocation mémoire, ou autre problème).
 * 
 * @note   L'appelant est responsable de libérer la mémoire allouée pour le tableau MD5 retourné.
 * 
 * @details 
 * - La fonction lit tout le contenu du fichier en mémoire, calcule le MD5 pour le fichier complet, 
 *   puis retourne le hash résultant.
 * - Si une erreur survient, un message d'erreur est écrit dans `stderr` et `NULL` est retourné.
 * - Les erreurs possibles incluent :
 *   - Fichier invalide ou NULL.
 *   - Problèmes liés à la détermination de la taille ou à la lecture du fichier.
 *   - Échec d'allocation de mémoire.
 */
unsigned char *md5_file(FILE *file){
    if (!file) {
        fprintf(stderr, "Paramètre invalides pour md5_file\n");
        return NULL;
    }
    
    // Allouer un buffer pour le résultat MD5
    unsigned char *md5_result = malloc(MD5_DIGEST_LENGTH);
    if (!md5_result) {
        perror("Erreur lors de l'allocation de mémoire pour md5_result");
        return NULL;
    }

    // Déterminer la taille du fichier
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Erreur lors du parcours du fichier");
        free(md5_result);
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size == -1) {
        perror("Erreur lors de la récupération de la taille du fichier");
        free(md5_result);
        return NULL;
    }

    rewind(file); // Revenir au début du fichier

    // Allouer un buffer pour lire le fichier


    unsigned char *file_buffer = malloc(file_size);
    if (!file_buffer) {
        perror("Erreur lors de l'allocation de mémoire pour le fichier");
        free(md5_result);
        return NULL;
    }


    // Lire le fichier entier
    size_t bytes_read = fread(file_buffer, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        perror("Erreur lors de la lecture du fichier");
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
 * 
 * @return size_t Nombre total de chunks traités (y compris les duplicatas). Retourne -1 en cas d'erreur.
 * 
 *  * @note 
 * - La taille de chaque chunk est définie par la constante `HASH_TABLE_SIZE`.
 * - Les buffers de données pour les chunks sont alloués dynamiquement. Il est de la responsabilité 
 *   de l'appelant de libérer ces buffers après utilisation.
 */
size_t deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table){
    // on détermine si le fichier est valide
    if (!file || !chunks || !hash_table ) {
        fprintf(stderr, "Paramètres invalides pour deduplicate_file\n");
        return -1;}

    // Déterminer la taille du fichier
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Erreur lors du parcours du fichier");
        return -1;
    }

    long file_size = ftell(file);
    if (file_size == -1) {
        perror("Erreur lors de la récupération de la taille du fichier");
        return -1;
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
            return -1;
        }

        // Lire les données du chunk
        size_t bytes_read = fread(buffer, 1, bytes_to_read, file);
        if (bytes_read != bytes_to_read) {
            perror("Erreur lors de la lecture du chunk");
            free(buffer);
            return -1;
        }

        // Calculer le MD5 du chunk
        unsigned char md5[MD5_DIGEST_LENGTH];
        compute_md5(buffer, bytes_read, md5);

        // Ajouter le MD5 dans la table de hachage
        add_md5(hash_table, md5, i);

        // Vérifier si le MD5 existe déjà dans la table de hachage
        int existing_index = find_md5(hash_table, md5);


        if (existing_index == -1) {
            memcpy(chunks[i].md5, md5, MD5_DIGEST_LENGTH);
            chunks[i].data = buffer;
        } else {
            chunks[i].data = (void *)(intptr_t)existing_index; // Stocker l'indice du chunk existant dans data
            memcpy(chunks[i].md5, md5, MD5_DIGEST_LENGTH);
            free(buffer);
        }


    }
    return chunk_count;
}



/**
 * @brief Fonction permettant de charger un fichier dédupliqué en table de chunks
 *        en remplaçant les références par les données correspondantes
 * 
 * @param file est le nom du fichier dédupliqué présent dans le répertoire de sauvegarde
 * @param chunks représente le tableau de chunk qui contiendra les chunks restauré depuis filename
 * @param chunk_count est un compteur du nombre de chunk restauré depuis le fichier filename
 */
void undeduplicate_file(FILE *file, Chunk **chunks, int *chunk_count) {
    // on détermine si le fichier est valide
    if (!file || !chunks || !chunk_count) {
        fprintf(stderr, "Paramètres invalides pour undeduplicate_file\n");
        return;}

    // Lire le nombre total de chunks depuis le fichier
    if (fread(chunk_count, sizeof(int), 1, file) != 1) {
        perror("Erreur lors de la lecture du compteur de chunks");
        return;
    }

    // Allouer de la mémoire pour les chunks
    *chunks = malloc(sizeof(Chunk) * (*chunk_count));
    if (!*chunks) {
        perror("Erreur d'allocation mémoire pour les chunks");
        return;
    }

    // Parcourir le fichier pour charger chaque chunk
    for (int i = 0; i < *chunk_count; i++) {
        // Lire le MD5 du chunk
        if (fread((*chunks)[i].md5, MD5_DIGEST_LENGTH, 1, file) != 1) {
            perror("Erreur lors de la lecture du MD5 du chunk");
            free(*chunks);
            *chunks = NULL;
            return;
        }

        // Lire la taille des données du chunk (0 pour un chunk référencé)
        size_t data_size;
        if (fread(&data_size, sizeof(size_t), 1, file) != 1) {

            perror("Erreur lors de la lecture de la taille du chunk");
            free(*chunks);
            *chunks = NULL;
            return;
        }

        if (data_size > 0) {
            // Chunk avec des données réelles
            (*chunks)[i].data = malloc(data_size);
            if (!(*chunks)[i].data) {
                perror("Erreur d'allocation mémoire pour les données du chunk");
                free(*chunks);
                *chunks = NULL;
                return;
            }

            // Lire les données du chunk
            if (fread((*chunks)[i].data, data_size, 1, file) != 1) {
                perror("Erreur lors de la lecture des données du chunk");
                free((*chunks)[i].data);
                free(*chunks);
                *chunks = NULL;
                return;
            }
        } else {
            // Chunk référencé : lire l'index du chunk source
            int referenced_index;
            if (fread(&referenced_index, sizeof(int), 1, file) != 1) {
                perror("Erreur lors de la lecture de l'indice référencé");
                free(*chunks);
                *chunks = NULL;
                return;
            }

            // Vérifier que l'indice référencé est valide
            if (referenced_index < 0 || referenced_index >= i) {
                fprintf(stderr, "Indice de chunk référencé invalide : %d\n", referenced_index);
                free(*chunks);
                *chunks = NULL;
                return;
            }

            // Copier les données du chunk référencé
            (*chunks)[i].data = (*chunks)[referenced_index].data;
        }
    }
}
