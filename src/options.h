#ifndef OPTIONS_H
#define OPTIONS_H

/**
 * @brief Structure contenant les options de sauvegarde et de restauration.
 *
 * Cette structure stocke les options configurées par l'utilisateur pour contrôler le comportement du programme, telles que les opérations de sauvegarde, restauration, et la liste des sauvegardes.
 * Elle inclut également des paramètres de connexion réseau (serveur source et destination), des options de mode "dry-run" et "verbose".
 *
 * @note Les options `--backup`, `--restore`, et `--list-backups` sont incompatibles entre elles.
 */
typedef struct {
    int backup_flag;         /**Active l'option de sauvegarde (1) ou non (0). */
    int restore_flag;        /**Active l'option de restauration (1) ou non (0). */
    int list_backups_flag;   /**Active l'option de liste des sauvegardes (1) ou non (0). */
    int dry_run_flag;        /**Active le mode "dry-run" (1) pour simuler les opérations. */
    int verbose_flag;        /**Active le mode "verbose" (1) pour un affichage détaillé. */

    char *d_server;          /**Adresse IP du serveur de destination. */
    int d_port;              /**Port du serveur de destination. */
    char *s_server;          /**Adresse IP du serveur source. */
    int s_port;              /**Port du serveur source. */
    char *dest_path;         /**Chemin du dossier de destination. */
    char *source_path;       /**Chemin du dossier source. */
} BackupOptions;

// Variable globale externe
extern BackupOptions options;

// Fonctions de gestion des options
void init_options(void);
void free_options(void);

#endif // OPTIONS_H
