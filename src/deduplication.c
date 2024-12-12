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
 * Si des collisions surviennent, elles ne sont pas gérées dans cette implémentation basique.
 * 
 * @param hash_table Le tableau de hachage qui contiendra les entrées MD5
 * @param md5 Le hash MD5 à ajouter dans la table de hachage
 * @param index L'index du chunk associé au hash MD5
 */
void add_md5(Md5Entry *hash_table, unsigned char *md5, int index) {
    // Calculer l'indice dans la table de hachage en utilisant la fonction de hachage
    unsigned int hash_index = hash_md5(md5);

    // Ajouter l'entrée dans la table à l'indice calculé
    // Copier le MD5 dans la table de hachage
    memcpy(hash_table[hash_index].md5, md5, MD5_DIGEST_LENGTH);

    // Enregistrer l'index du chunk dans la table
    hash_table[hash_index].index = index;
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
        fprintf(stderr, "Erreur : Fichier invalide pour le calcul du MD5\n");
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
 */
void deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table){
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



/**
 * @brief Fonction permettant de charger un fichier dédupliqué en table de chunks
 *        en remplaçant les références par les données correspondantes
 * 
 * @param file est le nom du fichier dédupliqué présent dans le répertoire de sauvegarde
 * @param chunks représente le tableau de chunk qui contiendra les chunks restauré depuis filename
 * @param chunk_count est un compteur du nombre de chunk restauré depuis le fichier filename
 */
void undeduplicate_file(FILE *file, Chunk **chunks, int *chunk_count) {
    
}
