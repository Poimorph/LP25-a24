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
    char * result = malloc(sizeof(char) * strlen(completePath));
    size_t length = strlen(repertoryPath);
    for (size_t i = length; i <= strlen(completePath); i++) {
        result[i - (length)] = completePath[i];
    }
    result[strlen(result)] = '\0';
    printf("%s\n", result);
    return result;
}

char * shortFirstDelimiter(char * path) {
    char * result = malloc(sizeof(char) * MAX_PATH);
    int isPassed = 0;
    int occurence = 0;

    for (size_t i = 0; i < strlen(path); i++) {
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

    for (size_t i = 0; i < strlen(path); i++) {
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

    time_t t = time(NULL); // On définit la variable date
    struct tm tm = *localtime(&t);
    char date[72];
    snprintf(date, sizeof(date), "%d-%02d-%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    DIR* dir = opendir(backup_dir); // On ouvre le dossier de sauvegarde. Et on cherche à savoir si il existe
    if (dir == NULL) {
        //Si il n'existe pas on le créer
        mkdir(backup_dir,0777);
    }

    char path[MAX_PATH + 17];

    snprintf(path, sizeof(path), "%s/.backup_log.txt", backup_dir); // On construit le chemin vers le dossier de backup


    if (access(path, F_OK) == -1) { //Si le fichier .backup_log n'existe pas on fait une backup complète

        //On construit et on créer le chemin vers le dossier de backup complète
        char complete_backup[MAX_PATH + 20];
        snprintf(complete_backup, sizeof(complete_backup), "%s/%s", backup_dir, date);
        mkdir(complete_backup,0777);

        // On copie tout les fichiers.
        copy_file(source_dir, complete_backup);

        FILE* backup_log = fopen(path, "wb");
        
        PathList *list_of_path = list_files(complete_backup);

        for (int i = 0; i < list_of_path->count; i++) { // Pour chaque  fichier ou dossier copé.
            log_element *log = malloc(sizeof(log_element));

            char backup[MAX_PATH + 1]; // On définit son chemin relatif par rapport au dossier de backup
            snprintf(backup, sizeof(backup), "%s/", backup_dir);
            char * full_path = PathSplitting(list_of_path->paths[i], backup);

            log->path = full_path;

            log->date = date;
            struct stat statbuff;

            stat(list_of_path->paths[i], &statbuff);
            printf("%d\n",stat(list_of_path->paths[i], &statbuff));
            printf("%d", S_ISDIR(statbuff.st_mode));
            if (S_ISREG(statbuff.st_mode)) { // Si le présupposé fichier est un fichier alors on calule son md5

                FILE* tempFile = fopen(list_of_path->paths[i], "rb");
                unsigned char * md5temp = md5_file(tempFile);
                for (int y = 0; y < MD5_DIGEST_LENGTH; y++) {
                    log->md5[y] = md5temp[y];
                }
            }
            write_log_element(log, path);
            free(log);
            if (S_ISREG(statbuff.st_mode)){
                backup_file(list_of_path->paths[i]); // Enfin on le déduplique.
            }
            
        }
        fclose(backup_log);
    } else {

        char incremental_backup[MAX_PATH + 73];
        //Je calcule le nombre d'élément présent dans le dossier, cela nous donneras à combien de backup nous serons (si on retir le fichier backup et le dossier de sauvegarde complète

        snprintf(incremental_backup, sizeof(incremental_backup), "%s/%s", backup_dir, date);
        mkdir(incremental_backup,0777);


        //Je copie le répértoire entier, ensuite pour chaque ficher je compare à la version originale, si c'est la même je supprime le fichier. Si le fichier est différent je le transforme en fichier backup.


        copy_file(source_dir, incremental_backup);

        PathList *current_file_list = list_files(incremental_backup);
        
        
        char incremental_final_backup[MAX_PATH + 74]; // A des fins d'organisation il est nécessaire de retirer le premier slash
        snprintf(incremental_final_backup, sizeof(incremental_final_backup), "%s/", incremental_backup);
        for (int i = 0; i < current_file_list->count; i++) {


            

            log_element * element = malloc(sizeof(log_element));
            
            
            
            //Calcul le chemin relatif
            char * relative = PathSplitting(current_file_list->paths[i], incremental_final_backup);
            
            
            
            element->path = relative;
            struct stat statbuff;
            stat(current_file_list->paths[i], &statbuff);
            
            
            if (S_ISREG(statbuff.st_mode)) {
                FILE* file = fopen(current_file_list->paths[i], "rb");
                unsigned char* md5 = md5_file(file);
                
                for (int y = 0; y < MD5_DIGEST_LENGTH; y++) {
                    element->md5[y] = md5[y];
                }
                
            }
            
            
            element->date = date;
            

            update_backup_log(element, path, date);

        }
        free(current_file_list);
        char backup_log_incremental[MAX_PATH + 89];
        snprintf(backup_log_incremental, sizeof(backup_log_incremental), "%s/.backup_log.txt", incremental_backup);
        FILE *d =fopen(backup_log_incremental,"wb");
            FILE *f =fopen(path,"rb");
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

    } 
    else {
        for (int i=0;i<chunk_count-1;i++){
            fwrite(chunks[i].data, 1, CHUNK_SIZE, file);
        }
        size_t actual_size = strlen((char *)chunks[chunk_count-1].data);
        actual_size= (actual_size>CHUNK_SIZE)?CHUNK_SIZE:actual_size;
        fwrite(chunks[chunk_count-1].data, 1, actual_size, file);
        fclose(file);
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

    char backup_log_path[MAX_PATH + 16];
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
        char completePath[MAX_PATH * 2]; // Je concatène les chemins pour avoir un chemin absolue sur les fichiers du backup_log
        snprintf(completePath, sizeof(completePath), "%s/%s", temp2,log->path);
        free(temp2);


        char restorePath[MAX_PATH*2];
        snprintf(restorePath, sizeof(restorePath), "%s/%s", restore_dir, shortFirstDelimiter((char*)log->path));

        struct stat statbuff;
        stat(completePath, &statbuff);
        if (S_ISDIR(statbuff.st_mode)) { // Si l'élément à restaurer est un dossier
            if (access(restorePath, F_OK) == -1) {// Et si il n'existe pas dans le dossier de restauration
                mkdir(restorePath, 0777);
            }
        } else {
            Chunk * chunks = NULL;
            int chunk_count = 0;
            FILE* file = fopen(completePath,"rb");
            if (!file) {
                perror("Could not open backup id");
                exit(EXIT_FAILURE);
            }
            undeduplicate_file(file, &chunks, &chunk_count);
            fclose(file);
            write_restored_file(restorePath, chunks, chunk_count);
        }

        if (log == logList->tail) {
            log = NULL;
        } else {
            log = log->next;
        }

    }


}
// Fonction permettant de lister les différentes sauvegardes présentes dans la destination
void list_backups(const char *backup_dir) {
    PathList* list_of_files = list_files(backup_dir);
    if (list_of_files == NULL) {
        perror("Could not list backup files");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Voici les backups disponibles : \n");
        for (int i = 0; i < list_of_files->count; i++) {
            char * temp_char = PathSplitting(list_of_files->paths[i], (char*)backup_dir);
            printf("%s\n", temp_char);
            free(temp_char);
        }
    }
}
