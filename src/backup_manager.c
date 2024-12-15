#include "backup_manager.h"
#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

char * PathSplitting(char * completePath, char * repertoryPath) {
    printf("%s\n", repertoryPath);
    printf("%s\n", completePath);
    char * result = malloc(sizeof(char) * 1024);
    int length = strlen(repertoryPath);
    for (int i = length; i <= strlen(completePath); i++) {
        result[i - (length)] = completePath[i];
    }
    result[strlen(result)] = '\0';
    return result;
}

char * shortFirstDelimiter(char * path) {
    char * result = malloc(sizeof(char) * 1024);
    int isPassed = 0;
    int occurence = 0;

    for (int i = 0; i < strlen(path); i++) {
        if (path[i] == '/' && isPassed == 0) {
            isPassed = 1;
        } else if (isPassed) {
            result[occurence] = path[i];
            occurence++;
        }
    }
    result[occurence] = '\0';
    return result;
}



// Fonction pour créer une nouvelle sauvegarde complète puis incrémentale
void create_backup(const char *source_dir, const char *backup_dir) {
    /* @param: source_dir est le chemin vers le répertoire à sauvegarder
    *          backup_dir est le chemin vers le répertoire de sauvegarde
    */
    printf("%s", backup_dir);
    struct stat stbuff;




    DIR* dir = opendir(backup_dir);
    if (dir == NULL) {
        //Si il n'existe pas on le créer
        mkdir(backup_dir,0777);
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s/.backup_log.txt", backup_dir);
    printf("%s", path);
    printf("%d", access(path, F_OK));

    if (access(path, F_OK) == -1) {

        char completeBackup[1024];
        snprintf(completeBackup, sizeof(completeBackup), "%s/%s", backup_dir, "fullbackup");
        mkdir(completeBackup,0777);

        copy_file(source_dir, completeBackup);

        FILE* backup_log = fopen(path, "w");

        PathList *listOfPath = list_files(completeBackup);

        for (int i = 0; i < listOfPath->count; i++) {
            log_element *log = malloc(sizeof(log_element));


            char pathOfFile[1024];
            strcpy(pathOfFile, listOfPath->paths[i]);

            char * shortpath[1024];
            strtok_r(listOfPath->paths[i], backup_dir, shortpath);

            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "fu%s", *shortpath);


            log->path = fullpath;

            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char date[1024];
            snprintf(date, sizeof(date), "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            log->date = date;
            struct stat statbuff;

            stat(pathOfFile, &statbuff);
            printf("%d\n",stat(pathOfFile, &statbuff));
            printf("%d", S_ISDIR(statbuff.st_mode));
            if (S_ISREG(statbuff.st_mode)) {

                FILE* tempFile = fopen(pathOfFile, "rb");
                unsigned char * md5temp = md5_file(tempFile);
                for (int y = 0; y < MD5_DIGEST_LENGTH; y++) {
                    log->md5[y] = md5temp[y];
                }
            }
            write_log_element(log, path);
            free(log);
        }
        fclose(backup_log);
    } else {

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

        PathList *CurrentFilelist = list_files(incrementalBackup);
        log_t *logList = read_backup_log(path);

        log_element *BackupLogElement = logList->head;
        char incrementalfinalbackup[1024];
        snprintf(incrementalfinalbackup, sizeof(incrementalfinalbackup), "%s/", incrementalBackup);
        for (int i = 0; i < CurrentFilelist->count; i++) {
            int isExisiting = 0;
            BackupLogElement = logList->head;
            char * relative = PathSplitting(CurrentFilelist->paths[i], incrementalfinalbackup);
            while (BackupLogElement != NULL) {
                char * Logelementpath = (char *)BackupLogElement->path;
                char * relativeLogElement = shortFirstDelimiter(Logelementpath);
                printf("Local : %s| Backup_log : %s\n", relative, relativeLogElement);
                //Si les fichiers sont les mêmes.
                if (strcmp(relative, relativeLogElement) == 0) {
                    isExisiting = 1;

                    struct stat statbuff;
                    stat(CurrentFilelist->paths[i], &statbuff);
                    if (S_ISREG(statbuff.st_mode)) {
                        FILE* fileLocal = fopen(CurrentFilelist->paths[i], "rb");
                        char backupfilePath[1024];
                        snprintf(backupfilePath, sizeof(backupfilePath), "%s/%s", backup_dir, relativeLogElement);


                        unsigned char * md5 = md5_file(fileLocal);


                        printf("%p\n", md5);
                        for (int y = 0; y < MD5_DIGEST_LENGTH; y++) {
                            printf("%02x", md5[y]);
                        }
                        printf("\n");
                        for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
                            printf("%02x", BackupLogElement->md5[j]);
                        }
                        printf("\n");
                        printf("%p\n", BackupLogElement->md5);


                        if (memcmp(md5, BackupLogElement->md5, MD5_DIGEST_LENGTH) == 0) {
                            remove(CurrentFilelist->paths[i]);
                            printf("has been removed");
                        } else {
                            Chunk * chunks;
                            Md5Entry *entry;
                            int chunk_count = (int)deduplicate_file(fileLocal, chunks, entry);
                            write_backup_file(CurrentFilelist->paths[i], chunks, chunk_count);
                            log_element* log_element = malloc(sizeof(log_element));
                            log_element->path = relative;
                            for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
                                log_element->md5[j] = md5[j];
                            }
                            time_t t = time(NULL);
                            struct tm tm = *localtime(&t);
                            char date[1024];
                            snprintf(date, sizeof(date), "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                            log_element->date = date;
                            update_backup_log(log_element, path);
                            free(log_element);

                        }

                    }

                }

                free(relativeLogElement);
                if (BackupLogElement == logList->tail) {
                    BackupLogElement = NULL;
                } else {
                    BackupLogElement = BackupLogElement->next;
                }



            }

            if (isExisiting == 0) {

            }

            free(relative);

        }
        char incrementalbackupfile[1024];
        snprintf(incrementalbackupfile, sizeof(incrementalbackupfile), "%s/.backup_log.txt", incrementalBackup);
        FILE *d =fopen(incrementalbackupfile,"w");
        FILE *f =fopen(path,"r");
        int c;
        while ((c = fgetc(f)) != EOF){
            fputc(c,d);
        }
        fclose(f);
        fclose(d);

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
    Chunk* chunks;
    Md5Entry* entry;
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
