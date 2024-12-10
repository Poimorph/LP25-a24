#include "backup_manager.h"
#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>



// Fonction pour créer une nouvelle sauvegarde complète puis incrémentale
void create_backup(const char *source_dir, const char *backup_dir) {
    /* @param: source_dir est le chemin vers le répertoire à sauvegarder
    *          backup_dir est le chemin vers le répertoire de sauvegarde
    */

    struct stat stbuff;
    //Checker si le dossier de backup existe
    if (stat(backup_dir, &stbuff) == -1) {
        //Si il n'existe pas on le créer
        mkdir(backup_dir);
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s/.backup_log", backup_dir);

    if (stat(path, &stbuff) == -1) {
        char completeBackup[1024];
        snprintf(completeBackup, sizeof(completeBackup), "%s/%s", backup_dir, "fullbackup");
        mkdir("fullbackup");
        copy_file(source_dir, completeBackup);

        FILE* backup_log = fopen(path, "w");

        int nbrOfFiles = 0;
        char ** listOfPath = list_files(source_dir, &nbrOfFiles);

        for (int i = 0; i < nbrOfFiles; i++) {
            log_element *log;
            log->path = listOfPath[i];
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char date[1024];
            snprintf(date, sizeof(date), "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            log->date = date;

        }






    } else {
        char incrementalBackup[1024];
        //Je calcule le nombre d'élément présent dans le dossier, cela nous donneras à combien de backup nous serons (si on retir le fichier backup et le dossier de sauvegarde complète
        DIR* dir = opendir(backup_dir);
        struct dirent* dp;
        int iteration = 0;
        while ((dp = readdir(dir)) != NULL) {
            iteration++;
        }
        closedir(dir);
        snprintf(incrementalBackup, sizeof(incrementalBackup), "%s/backup%d", backup_dir, iteration-1);
        mkdir(incrementalBackup);
        copy_file(source_dir, incrementalBackup);

        //Je copie le répértoire entier, ensuite pour chaque ficher je compare à la version originale, si c'est la même je supprime le fichier. Si le fichier est différent je le transforme en fichier backup.



    }



}

// Fonction permettant d'enregistrer dans fichier le tableau de chunk dédupliqué
void write_backup_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    /*
    */
    Chunk chunk[chunk_count];
    for (int i = 0; i < chunk_count; i++) {
        chunk[i] = chunks[i];
    }
    FILE* file = fopen(output_filename, "wb");
    if (!file) {
        perror("Cannto create %s\n");
    }
    else {
        fwrite(chunk, sizeof(Chunk), chunk_count, file);
    }
    fclose(file);



}


// Fonction implémentant la logique pour la sauvegarde d'un fichier
void backup_file(const char *filename) {
    /*
    */

    //Création des éléments pour la récéption des données
    Chunk* chunks = malloc(sizeof(Chunk)*1024);
    Md5Entry* entry  = malloc(sizeof(Md5Entry)*1024);
    FILE* file = fopen(filename, "rb");
    //Si le fichier n'éxiste pas
    if (!file) {
        printf("Could not open file %s\n", filename);
        exit(EXIT_FAILURE);
    } else {
        //On le déduplique
        deduplicate_file(file, chunks, entry);
        fclose(file);
        if (chunks != NULL) {
            //On écrit les chunks uniques dans le fichier de backup
            write_backup_file(filename,chunks, 1024);
        }
    }

}


// Fonction permettant la restauration du fichier backup via le tableau de chunk
void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    /*Ne fonctionne pas
    */

    FILE* file = fopen(output_filename, "wb");
    if (!file) {
        perror("Cannot open file");

    } else {

        Chunk **chunk = &chunks;
        undeduplicate_file(file, chunk, &chunk_count);
        fwrite(*chunk, sizeof(Chunk), chunk_count, file);

    }


}

// Fonction pour restaurer une sauvegarde
void restore_backup(const char *backup_id, const char *restore_dir) {
    /* @param: backup_id est le chemin vers le répertoire de la sauvegarde que l'on veut restaurer
    *          restore_dir est le répertoire de destination de la restauration
    */
}
