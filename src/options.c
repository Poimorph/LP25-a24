#include "options.h"

#include <stdlib.h>

// Définition de la variable globale
BackupOptions options;

void init_options(void) {
    // Initialiser tous les champs à 0/NULL
    options.backup_flag = 0;
    options.restore_flag = 0;
    options.list_backups_flag = 0;
    options.dry_run_flag = 0;
    options.verbose_flag = 0;
    options.d_server = NULL;
    options.d_port = -1;
    options.s_server = NULL;
    options.s_port = -1;
    options.dest_path = NULL;
    options.source_path = NULL;
}

void free_options(void) {
    // Libérer la mémoire allouée
    free(options.d_server);
    free(options.s_server);
    free(options.dest_path);
    free(options.source_path);
}
