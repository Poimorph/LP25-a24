#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include "file_handler.h"
#include "deduplication.h"

// Fonction permettant de lire un élément du fichier .backup_log
log_t *read_backup_log(const char *logfile) {
	/* Implémenter la logique pour la lecture d'une ligne du fichier ".backup_log"
    * @param: logfile - le chemin vers le fichier .backup_log
    * @return: une structure log_t
    */
	FILE *file = fopen(logfile, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return NULL;
    }

    log_t *log_list = malloc(sizeof(log_t));
    if (!log_list) {
        perror("Erreur lors de l'allocation de mémoire pour log_t");
        fclose(file);
        return NULL;
    }
    log_list->head = NULL;
    log_list->tail = NULL;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        // Supprimer le saut de ligne final
        line[strcspn(line, "\n")] = 0;

        // Diviser la ligne en 3 parties : chemin, md5 et date
        char *path = strtok(line, ";");
        unsigned char *md5_str = strtok(NULL, ";");
        char *date = strtok(NULL, ";");

        

        if (!path || !md5_str || !date) {
            fprintf(stderr, "Ligne invalide dans le fichier : %s\n", line);
            continue;
        }

        // Créer un nouvel élément de log
        log_element *new_element = malloc(sizeof(log_element));
        if (!new_element) {
            perror("Erreur lors de l'allocation de mémoire");
            fclose(file);
            return NULL;
        }

        new_element->path = strdup(path);
        if (!new_element->path) {
            perror("Erreur lors de la copie du chemin");
            free(new_element);
            fclose(file);
            return NULL;
        }

	// Copie directe de la chaîne MD5 en tant que tableau de caractères
        strncpy((char *)new_element->md5, md5_str, MD5_DIGEST_LENGTH);



        new_element->date = strdup(date);
        if (!new_element->date) {
            perror("Erreur lors de la copie de la date");
            free((char *)new_element->path);
            free(new_element);
            fclose(file);
            return NULL;
        }

        // Ajouter à la liste chaînée
        new_element->next = NULL;
        new_element->prev = log_list->tail;
        if (log_list->tail) {
            log_list->tail->next = new_element;
        } else {
            log_list->head = new_element;
        }
        log_list->tail = new_element;
    }

    fclose(file);
    return log_list;
}






// Fonction permettant de mettre à jour une ligne du fichier .backup_log
void update_backup_log(const log_element *element, const char *filename) {
    if (!element || !filename) {
        fprintf(stderr, "Paramètres invalides pour update_backup_log.\n");
        return;
    }

    FILE *file = fopen(filename, "r+");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier pour mise à jour");
        return;
    }

    char line[1024];
    long pos = 0;
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        // Sauvegarder la position actuelle du fichier
        pos = ftell(file);

        // Supprimer le saut de ligne final
        line[strcspn(line, "\n")] = 0;

        // Diviser la ligne en 3 parties : chemin, md5 et date
        char *path = strtok(line, ";");
        char *md5_str = strtok(NULL, ";");

        if (path && md5_str && strcmp(shortFirstDelimiter(path), shortFirstDelimiter(element->path))== 0) {
            found = 1;

            // Vérifier si le MD5 a changé
            char element_md5_str[MD5_DIGEST_LENGTH * 2 + 1] = {0};
            for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
                sprintf(&element_md5_str[i * 2], "%02x", element->md5[i]);
            }

            if (strcmp(md5_str, element_md5_str) != 0) {
                // Le MD5 a changé, mettre à jour la ligne
                fseek(file, pos - strlen(line) - 1, SEEK_SET);
                fprintf(file, "%s;", element->path);
		    fwrite(element->md5, 1, MD5_DIGEST_LENGTH, file);
                fprintf(file, ";%s\n", element->date);
            }
            break;
        }
    }

    if (!found) {
        // Ajouter le nouvel élément à la fin
        fseek(file, 0, SEEK_END);
        fprintf(file, "%s;", element->path);
	fwrite(element->md5, 1, MD5_DIGEST_LENGTH, file);
        fprintf(file, ";%s\n", element->date);
    }

    fclose(file);
}
/** 
   * @brief Implémenter la logique pour écrire un élément log de la liste chaînée log_element dans le fichier .backup_log
   * @param elt - un élément log à écrire sur une ligne
   * @param logfile - le chemin du fichier .backup_log
   */
void write_log_element( log_element *elt, const char *logfile) {
    
	if (!elt || !logfile) {
        fprintf(stderr, "Paramètres invalides pour write_log_element.\n");
        return;
    }
    FILE *file = fopen(logfile, "a"); // Ouvrir en mode ajout
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier pour écriture");
        return;
    }
    fprintf(file, "%s;", elt->path);
    fwrite(elt->md5, 1, MD5_DIGEST_LENGTH,file);
    fprintf(file, ";%s\n", elt->date);
    fclose(file);
}



// Fonction pour créer une nouvelle liste de chemins
PathList *create_pathlist() {
    PathList *list = malloc(sizeof(PathList));
    list->paths = malloc(10 * sizeof(char *)); // Initialisation avec une capacité de 10
    list->count = 0;
    list->capacity = 10;
    return list;
}

// Fonction pour ajouter un chemin à la liste dynamique
void add_path(PathList *list, const char *path) {
    // Si la liste est pleine, doubler sa capacité
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->paths = realloc(list->paths, list->capacity * sizeof(char *));
    }
    // Ajouter le chemin et incrémenter le compteur
    list->paths[list->count] = strdup(path);
    list->count++;
}

// Fonction pour libérer la mémoire allouée à la liste de chemins
void free_pathlist(PathList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->paths[i]); // Libérer chaque chemin
    }
    free(list->paths); // Libérer le tableau de chemins
    free(list); // Libérer la structure
}

// Fonction pour parcourir un répertoire et récupérer tous les chemins
PathList *list_files(const char *directory) {
    PathList *list = create_pathlist(); // Créer une nouvelle liste
    struct dirent *entry;
    DIR *dp = opendir(directory);

    if (dp == NULL) {
        perror("opendir"); // Afficher une erreur si le répertoire ne peut pas être ouvert
        free_pathlist(list);
        return NULL;
    }

    while ((entry = readdir(dp)) != NULL) {
        // Ignorer les entrées "." et ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Construire le chemin complet de l'entrée
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        // Ajouter le chemin à la liste
        add_path(list, path);

        // Vérifier si l'entrée est un répertoire
        struct stat info;
        if (stat(path, &info) == 0 && S_ISDIR(info.st_mode)) {
            // Appeler récursivement list_directory pour les sous-répertoires
            PathList *sublist = list_files(path);
            if (sublist) {
                // Ajouter les chemins du sous-répertoire à la liste principale
                for (int i = 0; i < sublist->count; i++) {
                    add_path(list, sublist->paths[i]);
                }
                free_pathlist(sublist); // Libérer la sous-liste
            }
        }
    }

    closedir(dp); // Fermer le répertoire
    return list;
}

void copy_file(const char *src, const char * dest) {
    /*C opie les fichiers d'une source à une autre
     * Alexis si tu pouvait gérer les sous-dossier et que j'aurais juste à mettre le dossier parent en paramètre et qu'il copie tout les
     * fichiers et sous-dossier en meme tempsd tu serais un amour*/
    struct dirent *entry;
    DIR *dp = opendir(src);
    if (dp == NULL) {

        perror("Erreur lors de l'ouverture du répertoire");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        // Ignorer les entrées "." et ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construire le chemin complet de l'entrée
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", src, entry->d_name);
	    char destpath[1024];
	    snprintf(destpath,sizeof(destpath), "%s/%s",dest, entry->d_name);

        // Si l'entrée est un répertoire, explorer récursivement
        struct stat entry_stat;

        if(stat(fullpath,&entry_stat)==-1){
            perror("Erreur lors de l'appel à stat");
            continue;
        }

        if (S_ISDIR(entry_stat.st_mode)) {
	    mkdir(destpath, 0777);  // Créer le dossier 
            copy_file(fullpath, destpath); // Recherche les sous dossier et fichiers
        }
	else{ // Si fichier, on copie le contenue du fichier source dans le fichier destination que l'on crée en meme temps 
	    FILE *d =fopen(destpath,"w");
	    FILE *f =fopen(fullpath,"r");
	    int c;
	    while ((c = fgetc(f)) != EOF){
		    fputc(c,d);
		    }
	    fclose(f);
	    fclose(d);
	}
}
}
