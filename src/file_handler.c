#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "file_handler.h"

void list_files(const char *path) {
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
        // Afficher l'entrée avec une indentation basée sur la profondeur
        printf("%s\n",fullpath);

        // Si l'entrée est un répertoire, explorer récursivement
        if (entry->d_type == DT_DIR) {
            // list_directory_recursive(fullpath);
        }
    }
}



char* read_file(const char *filepath, size_t *size) {
  FILE *file = fopen(filepath, "r"); // Ouvrir le fichier en mode lecture ("r")
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return NULL;
    }
    char buffer[1024]; // Tampon pour stocker chaque ligne lue
    printf("Contenu du fichier %s :\n", filepath);
    // Lire ligne par ligne
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }

    // Fermer le fichier
    fclose(file);
}

void write_file(const char *filepath, const void *data, size_t size) {
    FILE *file = fopen(filepath, "a"); // Ouvrir le fichier en mode ajout ("a")
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }
        if (fprintf(file, "%s\n", data) < 0) { // Écrire et vérifier si erreurs
            perror("Erreur lors de l'écriture dans le fichier");
            fclose(file);
            return;
        }
    printf("Données écrites avec succès dans le fichier %s\n", filepath);
    // Fermer le fichier
    fclose(file);
}
