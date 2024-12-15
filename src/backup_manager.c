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

    if (!isPassed) {
        free(result);
        return path;

    }
    result[occurence] = '\0';
    return result;
}

char * reversePath(char * path) {
    char * result = malloc(strlen(path)*sizeof(char));

    for (int i = 0; i < strlen(path); i++) {
        result[i] = path[strlen(path) - i - 1];
    }
    result[strlen(path)] = '\0';
    return result;
}

// Fonction pour créer une nouvelle sauvegarde complète puis incrémentale
void create_backup(const char *source_dir, const char *backup_dir) {
    /* @param: source_dir est le chemin vers le répertoire à sauvegarder
    *          backup_dir est le chemin vers le répertoire de sauvegarde
    */

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char date[1024];
    snprintf(date, sizeof(date), "%d-%02d-%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    DIR* dir = opendir(backup_dir);
    if (dir == NULL) {
        //Si il n'existe pas on le créer
        mkdir(backup_dir,0777);
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s/.backup_log.txt", backup_dir);


    if (access(path, F_OK) == -1) {

        char complete_backup[1024];
        snprintf(complete_backup, sizeof(complete_backup), "%s/%s", backup_dir, date);
        mkdir(complete_backup,0777);

        copy_file(source_dir, complete_backup);

        FILE* backup_log = fopen(path, "w");

        PathList *list_of_path = list_files(complete_backup);

        for (int i = 0; i < list_of_path->count; i++) {
            log_element *log = malloc(sizeof(log_element));

            char backup[1024];
            snprintf(backup, sizeof(backup), "%s/", backup_dir);
            char * full_path = PathSplitting(list_of_path->paths[i], backup);

            log->path = full_path;

            log->date = date;
            struct stat statbuff;

            stat(list_of_path->paths[i], &statbuff);
            printf("%d\n",stat(list_of_path->paths[i], &statbuff));
            printf("%d", S_ISDIR(statbuff.st_mode));
            if (S_ISREG(statbuff.st_mode)) {

                FILE* tempFile = fopen(list_of_path->paths[i], "rb");
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

        char incremental_backup[1024];
        //Je calcule le nombre d'élément présent dans le dossier, cela nous donneras à combien de backup nous serons (si on retir le fichier backup et le dossier de sauvegarde complète

        snprintf(incremental_backup, sizeof(incremental_backup), "%s/%s", backup_dir, date);
        mkdir(incremental_backup,0777);


        //Je copie le répértoire entier, ensuite pour chaque ficher je compare à la version originale, si c'est la même je supprime le fichier. Si le fichier est différent je le transforme en fichier backup.


        copy_file(source_dir, incremental_backup);

        PathList *current_file_list = list_files(incremental_backup);
        log_t *logList = read_backup_log(path);

        log_element *backup_log_element = logList->head;
        char incremental_final_backup[1024];
        snprintf(incremental_final_backup, sizeof(incremental_final_backup), "%s/", incremental_backup);
        for (int i = 0; i < current_file_list->count; i++) {



            int isExisiting = 0;
            backup_log_element = logList->head;
            char * relative = PathSplitting(current_file_list->paths[i], incremental_final_backup);

            while (backup_log_element != NULL) {
                char * log_element_path = (char *)backup_log_element->path;
                char * relative_log_element = shortFirstDelimiter(log_element_path);
                //Si les fichiers sont les mêmes.
                if (strcmp(relative, relative_log_element) == 0) {
                    isExisiting = 1;
                    struct stat statbuff;
                    stat(current_file_list->paths[i], &statbuff);
                    if (S_ISREG(statbuff.st_mode)) {
                        FILE* file_local = fopen(current_file_list->paths[i], "rb");
                        char backup_file_path[1024];
                        snprintf(backup_file_path, sizeof(backup_file_path), "%s/%s", backup_dir, relative_log_element);


                        unsigned char * md5 = md5_file(file_local);



                        for (int y = 0; y < MD5_DIGEST_LENGTH; y++) {
                            printf("%02x", md5[y]);
                        }
                        printf("\n");
                        for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
                            printf("%02x", backup_log_element->md5[j]);
                        }
                        printf("\n");



                        if (memcmp(md5, backup_log_element->md5, MD5_DIGEST_LENGTH) == 0) {

                            remove(current_file_list->paths[i]);
                            printf("has been removed");
                        } else {
                            printf("ici");

                            backup_file(current_file_list->paths[i]);

                            log_element* log = malloc(sizeof(log_element));
                            char backup[1024];
                            printf("backupppp : %s", backup_dir);
                            snprintf(backup, sizeof(backup), "%s/", backup_dir);
                            log->path = PathSplitting(current_file_list->paths[i], backup);
                            for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
                                log->md5[j] = md5[j];
                            }

                            log->date = date;
                            update_backup_log(log, path);


                        }

                    }

                }


                if (backup_log_element == logList->tail) {
                    backup_log_element = NULL;
                } else {
                    backup_log_element = backup_log_element->next;
                }



            }

            if (isExisiting == 0) {
                log_element *log = malloc(sizeof(log_element));

                char backup[1024];
                printf("backupppp : %s", backup_dir);
                snprintf(backup, sizeof(backup), "%s/", backup_dir);
                log->path = PathSplitting(current_file_list->paths[i], backup);


                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                char date[1024];
                snprintf(date, sizeof(date), "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                log->date = date;
                struct stat statbuff;

                stat(current_file_list->paths[i], &statbuff);
                if (S_ISREG(statbuff.st_mode)) {

                    FILE* tempFile = fopen(current_file_list->paths[i], "rb");
                    unsigned char * md5temp = md5_file(tempFile);
                    for (int y = 0; y < MD5_DIGEST_LENGTH; y++) {
                        log->md5[y] = md5temp[y];
                    }
                }
                write_log_element(log, path);

                free(log);
            }



        }
        char incrementalbackupfile[1024];
        snprintf(incrementalbackupfile, sizeof(incrementalbackupfile), "%s/.backup_log.txt", incremental_backup);
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

/**
 * @brief Enregistre un tableau de chunks dédupliqués dans un fichier de sauvegarde.
 * 
 * @param output_filename Le nom du fichier où les données doivent être sauvegardées.
 * @param chunks Le tableau de chunks à sauvegarder.
 * @param chunk_count Le nombre total de chunks dans le tableau.
 */
void write_backup_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    if (!output_filename || !chunks || chunk_count <= 0) {
        fprintf(stderr, "Paramètres invalides pour write_backup_file\n");
        return;
    }

    FILE *file = fopen(output_filename, "wb");
    if (!file) {
        perror("Erreur lors de la création du fichier de sauvegarde");
        return;
    }

    // Écrire le nombre total de chunks dans le fichier
    if (fwrite(&chunk_count, sizeof(int), 1, file) != 1) {
        perror("Erreur lors de l'écriture du compteur de chunks");
        fclose(file);
        return;
    }

    // Parcourir les chunks et enregistrer leurs données
    for(int i=0;i<chunk_count;i++){
        // Écriture du hash MD5
        if (fwrite(chunks[i].md5, MD5_DIGEST_LENGTH, 1, file) != 1) {
            perror("Erreur lors de l'écriture du MD5");
            fclose(file);
            return;
        }

        // Vérifier si le chunk contient des données réelles ou une référence
        if ((intptr_t)chunks[i].data < chunk_count) {
            // Chunk référencé : écrire une taille de données de 0 et l'index référencé
            size_t data_size = 0;
            int referenced_index = (int)(intptr_t)chunks[i].data;

            
            if (fwrite(&data_size, sizeof(size_t), 1, file) != 1 ||
                fwrite(&referenced_index, sizeof(int), 1, file) != 1) {
                perror("Erreur lors de l'écriture d'un chunk référencé");
                fclose(file);
                return;
            }
        } else {
            // Chunk avec des données réelles : écrire la taille et les données
            size_t data_size = strlen((char *)chunks[i].data) + 1; // Inclure le caractère nul
            if (fwrite(&data_size, sizeof(size_t), 1, file) != 1 ||
                fwrite(chunks[i].data, data_size, 1, file) != 1) {
                perror("Erreur lors de l'écriture d'un chunk réel");
                fclose(file);
                return;
            }
        }

    }
    
    fclose(file);
}


// Fonction implémentant la logique pour la sauvegarde d'un fichier
void backup_file(const char *filename) {
    /*
    */

    //Création des éléments pour la récéption des données
    struct stat statbuff;
    stat(filename, &statbuff);
    int nbr_chunk = 0;
    if (statbuff.st_size % CHUNK_SIZE == 0) {
        nbr_chunk = (int)statbuff.st_size / CHUNK_SIZE;
    } else {
        nbr_chunk = (int)statbuff.st_size / CHUNK_SIZE;
        nbr_chunk++;
    }

    Chunk * chunks = malloc(nbr_chunk * sizeof(Chunk));
    Md5Entry entry[HASH_TABLE_SIZE];
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
            write_backup_file(filename,chunks, nbr_chunk);
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
        fwrite(*chunk, sizeof(Chunk), chunk_count, file);

    }


}

// Fonction pour restaurer une sauvegarde
void restore_backup(const char *backup_id, const char *restore_dir) {
    /* @param: backup_id est le chemin vers le répertoire de la sauvegarde que l'on veut restaurer
    *          restore_dir est le répertoire de destination de la restauration
    */


    DIR *dir = opendir(backup_id); //Ouvrir le dossier de restauration
    if (dir == NULL) {
        perror("Could not open backup id");
        exit(EXIT_FAILURE);
    }

    char backup_log_path[1024];
    snprintf(backup_log_path, sizeof(backup_log_path), "%s/.backup_log.txt", backup_id); // Construction du chemin menant au backup_log
    log_t * logList = read_backup_log(backup_log_path); // Je récupère chaque fichier.
    if (logList == NULL) {
        perror("Could not read backup log");
        exit(EXIT_FAILURE);
    }
    log_element * log = logList->head;
    while (log != NULL) {

        char * temp = reversePath((char*)backup_id);
        char *temp1 = shortFirstDelimiter(temp);
        free(temp);
        char *temp2 = reversePath(temp1);
        free(temp1);
        char completePath[1024]; // Je concatène les chemins pour avoir un chemin absolue sur les fichiers du backup_log
        snprintf(completePath, sizeof(completePath), "%s/%s", temp2,log->path);
        free(temp2);

        printf("complete path : %s\n", completePath);

        char restorePath[1024];

        snprintf(restorePath, sizeof(restorePath), "%s/%s", restore_dir, shortFirstDelimiter((char*)log->path));

        struct stat statbuff;
        stat(completePath, &statbuff);
        if (S_ISDIR(statbuff.st_mode)) { // Si l'élément à restaurer est un dossier
            if (access(restorePath, F_OK) == -1) {// Et si il n'existe pas dans le dossier de restauration
                mkdir(restorePath, 0777);
            }
        }

        Chunk * chunks = NULL;
        int chunk_count = 0;
        FILE* file = fopen(completePath,"rb");
        if (!file) {
            perror("Could not open backup id");
            exit(EXIT_FAILURE);
        }
        undeduplicate_file(file, &chunks, &chunk_count);
        fclose(file);
        FILE*file1 = fopen(restorePath,"wb");
        for (int i = 0; i < chunk_count; i++) {
            fprintf(file1, "%s", (char *)chunks[i].data);
        }


        if (log == logList->tail) {
            log = NULL;
        } else {
            log = log->next;
        }

    }


}
