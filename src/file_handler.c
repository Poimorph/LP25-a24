#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include "file_handler.h"
#include "deduplication.h"

// Fonction permettant de lire un élément du fichier .backup_log
log_t read_backup_log(const char *logfile){
    /* Implémenter la logique pour la lecture d'une ligne du fichier ".backup_log"
    * @param: logfile - le chemin vers le fichier .backup_log
    * @return: une structure log_t
    */
}

// Fonction permettant de mettre à jour une ligne du fichier .backup_log
void update_backup_log(const char *logfile, log_t *logs){
  /* Implémenter la logique de modification d'une ligne du fichier ".bakcup_log"
  * @param: logfile - le chemin vers le fichier .backup_log
  *         logs - qui est la liste de toutes les lignes du fichier .backup_log sauvegardée dans une structure log_t
  */

}

void write_log_element(log_element *elt, FILE *logfile){
  /* Implémenter la logique pour écrire un élément log de la liste chaînée log_element dans le fichier .backup_log
   * @param: elt - un élément log à écrire sur une ligne
   *         logfile - le chemin du fichier .backup_log
   */
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
        struct stat *entry_stat;
        if(stat(fullpath,entry_stat)==-1){
            perror("Erreur lors de l'appel à stat");
            continue;
        }
        
        if (S_ISDIR(entry_stat->st_mode)) {
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


