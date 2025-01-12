#ifndef DEDUPLICATION_H
#define DEDUPLICATION_H

#include <dirent.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"

// Taille d'un chunk (4096 octets)
#define CHUNK_SIZE 4096

// Taille de la table de hachage qui contiendra les chunks
// dont on a déjà calculé le MD5 pour effectuer les comparaisons
#define HASH_TABLE_SIZE 1000

/**
 * @brief Structure pour représenter un chunk de données.
 *
 * @param md5 Hachage MD5 du chunk de données. Utilisé pour identifier de manière unique le chunk.
 * @param data Pointeur vers les données du chunk. Ce champ contient les données elles-mêmes du chunk.
 */
typedef struct {
    unsigned char md5[MD5_DIGEST_LENGTH]; /**< MD5 du chunk. */
    void *data;                           /**< Données du chunk. */
} Chunk;

// Table de hachage pour stocker les MD5 et leurs index
/**
 * @brief Structure pour une entrée dans la table de hachage des MD5.
 *
 * @param md5 Hachage MD5 du chunk. Utilisé pour identifier de manière unique le chunk.
 * @param index Index du chunk dans le tableau des chunks. Permet de retrouver le chunk correspondant.
 */
typedef struct {
    unsigned char md5[MD5_DIGEST_LENGTH];
    int index;
} Md5Entry;

/**
 * @brief Fonction de hachage MD5 pour l'indexation dans la table de hachage
 *
 * @param md5 Tableau de taille MD5_DIGEST_LENGTH
 * @return unsigned int - L'index calculé dans la plage [0, HASH_TABLE_SIZE - 1].
 */
unsigned int hash_md5(unsigned char *md5);

/**
 * @brief Fonction pour calculer le MD5 d'un chunk.
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
void compute_md5(void *data, size_t len, unsigned char *md5_out);

/**
 * @brief Fonction permettant de chercher un MD5 dans la table de hachage
 *
 * @param hash_table le tableau de hachage qui contient les MD5 et l'index des chunks unique
 * @param md5 le md5 du chunk dont on veut déterminer l'unicité
 * @return retourne l'index s'il trouve le md5 dans le tableau et -1 sinon
 */
int find_md5(Md5Entry *hash_table, unsigned char *md5);

/**
 * @brief Fonction pour ajouter un MD5 dans la table de hachage
 *
 * @param hash_table le tableau de hachage qui contient les MD5 et l'index des chunks unique
 * @param md5 le md5 du chunk dont on veut déterminer l'unicité
 * @return retourne l'index s'il trouve le md5 dans le tableau et -1 sinon
 */
void add_md5(Md5Entry *hash_table, unsigned char *md5, int index);

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
size_t deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table);

/**
 * @brief Fonction permettant de charger un fichier dédupliqué en table de chunks
 *        en remplaçant les références par les données correspondantes
 *
 * @param file est le nom du fichier dédupliqué présent dans le répertoire de sauvegarde
 * @param chunks représente le tableau de chunk qui contiendra les chunks restauré depuis le fichier file
 * @param chunk_count est un compteur du nombre de chunk restauré depuis le fichier file
 */
void undeduplicate_file(FILE *file, Chunk **chunks, int *chunk_count);

/**
 * @brief  Fonction pour calculer le MD5 d'un fichier
 *
 * @param  file Pointeur vers un fichier ouvert en mode binaire (FILE*), dont le MD5 sera calculé.
 *
 * @return unsigned char* - Un tableau de 16 octets contenant le hash MD5 du fichier.
 *                          Renvoie NULL en cas d'erreur (fichier invalide, allocation mémoire, ou autre problème).
 *
 * @note   L'appelant est responsable de libérer la mémoire allouée pour le tableau MD5 retourné.
 *
 */
unsigned char *md5_file(FILE *file);

#endif // DEDUPLICATION_H
