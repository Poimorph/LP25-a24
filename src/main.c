#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "file_handler.h"
#include "deduplication.h"
#include "backup_manager.h"
#include "network.h"




int main(int argc, char *argv[]) {
    // tests :
    // testMD5Compute();
    printf("%s", argv[1]);
    create_backup(argv[1], argv[2]);
    // Analyse des arguments de la ligne de commande

    // Implémentation de la logique de sauvegarde et restauration
    // Exemples : gestion des options --backup, --restore, etc.
    
    return EXIT_SUCCESS;
}

