#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <dirent.h>


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
     *         md5_out  - buffer de sortie (doit être au moins MD5_DIGEST_LENGTH octets)
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
    // md5_out contient maintenant le hachage MD5 des données
    EVP_MD_CTX_free(ctx);
}

// Fonction permettant de chercher un MD5 dans la table de hachage
int find_md5(Md5Entry *hash_table, unsigned char *md5) {
    /* @param: hash_table   est le tableau de hachage qui contient les MD5 et l'index des chunks unique
     *         md5          est le md5 du chunk dont on veut déterminer l'unicité
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

unsigned char *md5_file(FILE *file){
    unsigned char md5[MD5_DIGEST_LENGTH];
    if (!file) {
        fprintf(stderr, "Fichier invalide pour le calcul du MD5\n");
        return NULL;
    }

}

// Fonction pour convertir un fichier non dédupliqué en tableau de chunks
void deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table){
    /* @param:  file        est le fichier qui sera dédupliqué
    *           chunks      est le tableau de chunks initialisés qui contiendra les chunks issu du fichier
    *           hash_table  est le tableau de hachage qui contient les MD5 et l'index des chunks unique
    */
}


// Fonction permettant de charger un fichier dédupliqué en table de chunks
// en remplaçant les références par les données correspondantes
void undeduplicate_file(FILE *file, Chunk **chunks, int *chunk_count) {
    /* @param: file         est le nom du fichier dédupliqué présent dans le répertoire de sauvegarde
    *          chunks       représente le tableau de chunk qui contiendra les chunks restauré depuis filename
    *          chunk_count  est un compteur du nombre de chunk restauré depuis le fichier filename
    */
}
