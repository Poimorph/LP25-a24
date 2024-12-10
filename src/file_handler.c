#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
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

char ** list_files(const char *path){
  /* Implémenter la logique pour lister les fichiers présents dans un répertoire, récursive elle doit rencoyer un tableau à double dimension des chemins des différents fichiers
  */
}

void copy_file(const char *src, const char * dest) {
    /*Copie les fichiers d'une source à une autre
     * Alexis si tu pouvait gérer les sous-dossier et que j'aurais juste à mettre le dossier parent en paramètre et qu'il copie tout les
     * fichiers et sous-dossier en meme tempsd tu serais un amour*/
    struct dirent *entry;
    DIR *dp = opendir(path);
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
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
	    char destpath[1024];
	    snprintf(destpath,sizeof(destpath), "%s/%s",dest, entry->d_name);
        // Afficher l'entrée avec une indentation basée sur la profondeur
        printf("%s, %s \n",fullpath,destpath);

        // Si l'entrée est un répertoire, explorer récursivement
        if (entry->d_type == DT_DIR) {
	    mkdir(destpath, 0777);  // Créer le dossier 
            list_directory_recursive(fullpath, dest); // Recherche les sous dossier et fichiers
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

}
