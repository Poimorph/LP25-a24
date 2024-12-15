#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "file_handler.h"
#include "deduplication.h"
#include "backup_manager.h"
#include "network.h"

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



int main(int argc, char *argv[]) {
    // tests :

    create_backup(argv[1], argv[2]);
    
    // Analyse des arguments de la ligne de commande

    // Implémentation de la logique de sauvegarde et restauration
    // Exemples : gestion des options --backup, --restore, etc.

    // initialisation des options
    // BackupOptions options = {0};
    //
    // // définition des options longues
    // struct option long_options[] = {
    //     {"backup",        no_argument,       &options.backup_flag,        1},
    //     {"restore",       no_argument,       &options.restore_flag,       1},
    //     {"list-backups",  no_argument,       &options.list_backups_flag,  1},
    //     {"dry-run",       no_argument,       &options.dry_run_flag,       1},
    //     {"verbose",       no_argument,       &options.verbose_flag,       1},
    //     {"d-server",      required_argument, 0,                           'd'},
    //     {"d-port",        required_argument, 0,                           'D'},
    //     {"s-server",      required_argument, 0,                           's'},
    //     {"s-port",        required_argument, 0,                           'S'},
    //     {"dest",          required_argument, 0,                           'e'},
    //     {"source",        required_argument, 0,                           'o'},
    //     {0, 0, 0, 0}
    // };
    //
    //
    //
    // // Gestions des options
    // int option_index = 0;
    // int c;
    // while ((c = getopt_long(argc, argv, "vd:D:s:S:e:o:", long_options, &option_index)) != -1) {
    //     switch (c) {
    //         case 0:
    //             break;
    //         case 'v':
    //             options.verbose_flag = 1;
    //             break;
    //         case 'd':
    //             printf("Destination server IP: %s (NETWORK FEATURE NOT YET COMPLETED)\n", optarg);
    //             options.d_server = optarg;
    //             break;
    //         case 'D':
    //             printf("Destination server port: %s (NETWORK FEATURE NOT YET COMPLETED)\n", optarg);
    //             options.d_port = atoi(optarg);
    //             break;
    //         case 's':
    //             printf("Source server IP: %s (NETWORK FEATURE NOT YET COMPLETED)\n", optarg);
    //             options.s_server = optarg;
    //             break;
    //         case 'S':
    //             printf("Source server port: %s (NETWORK FEATURE NOT YET COMPLETED)\n", optarg);
    //             options.s_port = atoi(optarg);
    //             break;
    //         case 'e':
    //             printf("Destination path: %s\n", optarg);
    //             options.dest_path = optarg;
    //             break;
    //         case 'o':
    //             printf("Source path: %s\n", optarg);
    //             options.source_path = optarg;
    //             break;
    //         default:
    //             print_usage(argv[0]);
    //             return 1;
    //     }
    // }
    //
    // // Vérifie et affiche l'opération sélectionnée
    // if (options.backup_flag) {
    //     printf("Opération de sauvegarde sélectionnée\n");
    // } else if (options.restore_flag) {
    //     printf("Opération de restauration sélectionnée\n");
    // } else if (options.list_backups_flag) {
    //     printf("Opération de liste des sauvegardes sélectionnée\n");
    // }
    //
    // // Vérification et affichage du mode dry_run (simulation sans exécution réelle)
    // if (options.dry_run_flag) {
    //     printf("Dry run mode enabled\n");
    // }
    //
    // // Vérification et affichage du mode verbose (affichage détaillé)
    // if (options.verbose_flag) {
    //     printf("Verbose mode enabled\n");
    // }

    return EXIT_SUCCESS;
}

