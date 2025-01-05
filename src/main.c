#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "file_handler.h"
#include "deduplication.h"
#include "backup_manager.h"
#include "network.h"
#include "options.h"




void print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  --backup       Effectuer une sauvegarde\n");
    printf("  --restore      Restaurer une sauvegarde\n");
    printf("  --list-backups Lister les sauvegardes existantes\n");
    printf("  --dry-run      Simuler les opérations sans les exécuter\n");
    printf("  --verbose      Afficher des informations détaillées\n");
    printf("  --d-server IP  Adresse IP du serveur de destination\n");
    printf("  --d-port PORT  Port du serveur de destination\n");
    printf("  --s-server IP  Adresse IP du serveur source\n");
    printf("  --s-port PORT  Port du serveur source\n");
    printf("  --dest PATH    Chemin du dossier de destination\n");
    printf("  --source PATH  Chemin du dossier source\n");
}



int main(int argc, char *argv[]) {
    // tests :

    // create_backup(argv[1], argv[2]);
    // restore_backup("/home/vboxuser/CLionProjects/LP25-a24/src/dest/backup58", "/home/vboxuser/CLionProjects/LP25-a24/src/source");
    // Analyse des arguments de la ligne de commande

    // Implémentation de la logique de sauvegarde et restauration
    // Exemples : gestion des options --backup, --restore, etc.

    // Initialisation des options
    init_options();


    // // définition des options longues
    struct option long_options[] = {
        {"backup",        no_argument,       &options.backup_flag,        1},
        {"restore",       no_argument,       &options.restore_flag,       1},
        {"list-backups",  no_argument,       &options.list_backups_flag,  1},
        {"dry-run",       no_argument,       &options.dry_run_flag,       1},
        {"verbose",       no_argument,       &options.verbose_flag,       1},
        {"d-server",      required_argument, 0,                           'd'},
        {"d-port",        required_argument, 0,                           'D'},
        {"s-server",      required_argument, 0,                           's'},
        {"s-port",        required_argument, 0,                           'S'},
        {"dest",          required_argument, 0,                           'e'},
        {"source",        required_argument, 0,                           'o'},
        {0, 0, 0, 0}
    };



    // Gestions des options
    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "vd:D:s:S:e:o:", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                break;
            case 'v':
                options.verbose_flag = 1;
                break;
            case 'd':
                printf("Destination server IP: %s (NETWORK FEATURE NOT YET COMPLETED)\n", optarg);
                options.d_server = strdup(optarg);
                break;
            case 'D':
                printf("Destination server port: %s (NETWORK FEATURE NOT YET COMPLETED)\n", optarg);
                options.d_port = atoi(optarg);
                break;
            case 's':
                printf("Source server IP: %s (NETWORK FEATURE NOT YET COMPLETED)\n", optarg);
                options.s_server = strdup(optarg);
                break;
            case 'S':
                printf("Source server port: %s (NETWORK FEATURE NOT YET COMPLETED)\n", optarg);
                options.s_port = atoi(optarg);
                break;
            case 'e':
                printf("Destination path: %s\n", optarg);
                options.dest_path = strdup(optarg);
                break;
            case 'o':
                printf("Source path: %s\n", optarg);
                options.source_path = strdup(optarg);
                break;
            default:
                print_usage(argv[0]);
                free_options();
                return 1;
        }
    }


    // gestion des erreurs :

    // Vérification du nombre d'options mutuellement exclusives
    int options_index = options.backup_flag + options.restore_flag + options.list_backups_flag;

    if (options_index > 1) {
        fprintf(stderr, "Erreur : Vous ne pouvez spécifier qu'une seule action principale (backup, restore, list-backups)\n");
        print_usage(argv[0]);
        free_options();
        return 1;
    }

    // Si aucune action principale n'est spécifiée
    if (options_index == 0) {
        fprintf(stderr, "Erreur : Aucune action principale spécifiée\n");
        print_usage(argv[0]);
        free_options();
        return 1;
    }



    //if (options.s_port <= 0 || options.s_port > 65535) {
        //fprintf(stderr, "Erreur : Numéro de port source invalide\n");
        //free_options();
        //return 1;
    //}

    if (options.backup_flag) {
        if (!options.source_path || !options.dest_path) {
            fprintf(stderr, "Erreur : Les chemins source et destination sont requis pour la sauvegarde\n");
            free_options();
            return 1;
        }

        if ((options.d_port < 0 && options.d_port != -1) || options.d_port > 65535) {
            fprintf(stderr, "Erreur : Numéro de port destination invalide\n");
            free_options();
            return 1;
        }
        if (options.d_port == -1) {
            // Nous lançons en local
            printf("lolilolilo");
            printf("Exécution de la sauvegarde de %s vers %s\n",options.source_path, options.dest_path);
            create_backup(options.source_path,options.dest_path, NULL, NULL);
        } else {
            // Nous lançons en réseau
            printf("aaaaaaaaaaaaaa : %s", options.d_server);
            printf("Exécution de la sauvegarde de %s vers %s\n",options.source_path, options.dest_path);
            create_backup(options.source_path,options.dest_path, options.d_server, options.d_port);
        }

    }

    if (options.restore_flag) {
        if (!options.source_path || !options.dest_path) {
            fprintf(stderr, "Erreur : Les chemins source et destination sont requis pour la restauration\n");
            free_options();
            return 1;
        }

        printf("Restauration de %s vers %s\n",options.source_path, options.dest_path);
        restore_backup(options.source_path,options.dest_path);

    }

    if (options.list_backups_flag) {
        if (!options.dest_path) {
            fprintf(stderr, "Erreur : Le chemin destination est requis pour la restauration\n");
            free_options();
            return 1;
        }
        list_backups(options.dest_path);

    }


    // Libération de la mémoire
    free_options();
    return EXIT_SUCCESS;
}
