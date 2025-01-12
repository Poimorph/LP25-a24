#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "deduplication.h"
#include "file_handler.h"
#include "options.h"

// Taille Maximale d'un chemin
#define MAX_PATH 1024


/**
 * @brief Fonction pour retirer le premier dossier d'un chemin
 *
 * @param path chemin à traiter
 * @return char* chemin sans le premier dossier
 */
char *short_first_delimiter(char *path);

/**
 * @brief Fonction pour inverser un chemin
 *
 * @param path chemin à inverser
 * @return char* chemin inversé
 */
char *reverse_path(char *path);

/**
 * @brief Fonction pour séparer un chemin complet en un chemin relatif
 *
 * @param complete_path chemin complet
 * @param repertory_path chemin du répertoire
 * @return char* chemin relatif
 */
char *path_splitting(char *complete_path, char *repertory_path);

/**
 * @brief Fonction pour créer une nouvelle sauvegarde complète puis incrémentale
 *
 * @param source_dir chemin vers le répertoire à sauvegarder
 * @param backup_dir chemin vers le répertoire de sauvegarde
 */
void create_backup(const char *source_dir, const char *backup_dir);

/**
 * @brief Fonction pour restaurer une sauvegarde
 *
 * @param backup_id chemin vers le répertoire de la sauvegarde à restaurer
 * @param restore_dir chemin vers le répertoire de destination de la restauration
 */
void restore_backup(const char *backup_id, const char *restore_dir);

/**
 * @brief Fonction permettant la restauration du fichier backup via le tableau de chunk
 *
 * @param output_filename nom du fichier de sortie
 * @param chunks tableau de `Chunk`
 * @param chunk_count nombre de chunks
 */
void write_backup_file(const char *output_filename, Chunk *chunks, int chunk_count);

/**
 * @brief Fonction pour la sauvegarde de fichier dédupliqué
 *
 * @param filename nom du fichier à sauvegarder
 */
void backup_file(const char *filename);

/**
 * @brief Fonction permettant la restauration du fichier backup via le tableau de chunk
 *
 * @param output_filename Nom du fichier restauré
 * @param chunks Tableau de `Chunk`
 * @param chunk_count Nombre de chunks
 */
void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count);
/**
 * @brief Fonction permettant de lister les différentes sauvegardes présentes dans la destination
 *
 * @param backup_dir Répertoire de sauvegarde
 */
void list_backups(const char *backup_dir);

#endif // BACKUP_MANAGER_H
