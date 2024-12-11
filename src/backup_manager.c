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
    printf("%s", backup_dir);
    struct stat stbuff;
    //Checker si le dossier de backup existe


    DIR* dir = opendir(backup_dir);
    if (dir == NULL) {
        //Si il n'existe pas on le créer
        mkdir(backup_dir,0777);
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s/.backup_log.txt", backup_dir);
    stat(path, &stbuff);
    FILE* backup = fopen(path, "r");

    if (!backup) {
        fclose(backup);
        char completeBackup[1024];
        snprintf(completeBackup, sizeof(completeBackup), "%s/%s", backup_dir, "/fullbackup");
        mkdir(completeBackup,0777);
        printf("lol");
        copy_file(source_dir, completeBackup);
        printf("lol21");
        FILE* backup_log = fopen(path, "w");

        PathList *listOfPath = list_files(completeBackup);

        for (int i = 0; i < listOfPath->count; i++) {
            log_element *log = malloc(sizeof(log_element));

            char * token = strtok(listOfPath->paths[i], backup_dir);
            log->path = token;
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char date[1024];
            snprintf(date, sizeof(date), "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            log->date = date;
            fwrite(log, sizeof(log_element), 1, backup_log);
            free(log);
        }

        fclose(backup_log);






    } else {
        fclose(backup);
        char incrementalBackup[1024];
        //Je calcule le nombre d'élément présent dans le dossier, cela nous donneras à combien de backup nous serons (si on retir le fichier backup et le dossier de sauvegarde complète
        DIR* dir = opendir(backup_dir);
        struct dirent* dp;
        int iteration = 0;
        while ((dp = readdir(dir)) != NULL) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
                continue;
            }
            iteration++;
        }
        closedir(dir);
        snprintf(incrementalBackup, sizeof(incrementalBackup), "%s/backup%d", backup_dir, iteration-1);
        mkdir(incrementalBackup,0777);


        //Je copie le répértoire entier, ensuite pour chaque ficher je compare à la version originale, si c'est la même je supprime le fichier. Si le fichier est différent je le transforme en fichier backup.

        copy_file(source_dir, incrementalBackup);
        PathList *list = list_files(incrementalBackup);
        log_t logList = read_backup_log(path);
        log_element *element = logList.head;
        while (element != NULL) {
            char * elementpath = element->path;
            int isExisiting = 0;
            for (int i = 0; i < list->count; i++) {
                char * relativeIncrementalPath = strtok(list->paths[i], incrementalBackup);
                int y = 0;
                char relativeLastBackupPath[1024];
                char * token = strtok(elementpath, "/");

                while ((token = strtok(NULL, "/"))){
                    if (relativeLastBackupPath[0] == '\0') {
                        snprintf(relativeLastBackupPath, sizeof(relativeLastBackupPath), "%s/", token);
                    }else {
                        snprintf(relativeLastBackupPath, sizeof(relativeLastBackupPath), "%s/%s", relativeLastBackupPath, token);

                    };
                }
                if (strcmp(relativeLastBackupPath, relativeIncrementalPath) == 0) {
                    isExisiting = 1;
                    FILE* file1 = fopen(list->paths[i], "rb");
                    char backupfilePath[1024];
                    snprintf(backupfilePath, sizeof(backupfilePath), "%s/%s", backup_dir, elementpath);
                    FILE* file = fopen(backupfilePath, "rb");
                    if (strcmp(md5_file(file1), element->md5) == 0) {
                        remove(list->paths[i]);
                    } else {
                        Chunk * chunks;
                        Md5Entry *entry;
                        deduplicate_file(file1, chunks, entry);
                        int m = 0;

                        // write_backup_file(file1, chunks, );
                    }
                }
            }
        }

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
