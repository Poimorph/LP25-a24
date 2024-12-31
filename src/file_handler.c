#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include "file_handler.h"

#include "backup_manager.h"

#include "deduplication.h"
/**
 * @brief Convertit un caractère hexadécimal en valeur entière.
 *
 * @param c Le caractère hexadécimal à convertir.
 * @return La valeur entière correspondante ou -1 si invalide.
 */
int hex_to_int(unsigned char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';

    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

/**
 * @brief Convertit une chaîne hexadécimale MD5 en tableau de bytes.
 *
 * @param hex_md5 La chaîne de caractères représentant le MD5 en hexadécimal (32 caractères).
 * @param md5_bytes Le tableau de bytes qui recevra le MD5 converti (16 octets).
 *
 * @note La fonction vérifie si les caractères hexadécimaux sont valides, et si ce n'est pas le cas,
 *       elle affiche un message d'erreur et retourne sans effectuer la conversion.
 */
void md5_hex_to_bytes(unsigned char * hex_md5, unsigned char * md5_bytes) {

    // if (strlen(hex_md5) != 32) {
    //     fprintf(stderr, "md5_hex_to_bytes: Invalid MD5 string\n");
    //     return;
    // }
    for (int i = 0; i < 16; i++) {
        int high = hex_to_int(hex_md5[i * 2]);
        int low = hex_to_int(hex_md5[i * 2 + 1]);
        if (high == -1 || low == -1) {
            fprintf(stderr, "md5_hex_to_bytes: Invalid MD5 string\n");
            return;
        }
        md5_bytes[i] = (high * 16) + low;
    }
}

/**
 * @brief Lit un fichier de journal de sauvegarde et retourne une liste chaînée d'éléments de log.
 *
 * Cette fonction lit un fichier `.backup_log`, extrait les informations contenues dans chaque ligne
 * (un chemin, un hash MD5 et une date) et crée une liste chaînée d'éléments de log correspondant à
 * chaque ligne du fichier. Le fichier est ouvert en mode lecture, et chaque ligne est analysée et divisée
 * en trois parties : chemin, MD5 et date.
 *
 * @param logfile Le chemin vers le fichier `.backup_log` à lire.
 *
 * @return Une liste chaînée de `log_t` contenant les éléments de log extraits du fichier.
 *         Si une erreur se produit lors de l'ouverture du fichier ou de l'allocation mémoire,
 *         NULL est retourné.
 *
 * @note La fonction alloue de la mémoire pour chaque élément de log et pour les chaînes de caractères
 *       (chemin et date). La fonction attend que chaque ligne du fichier ait une structure spécifique
 *       avec trois parties séparées par un point-virgule (`;`).
 */
log_t *read_backup_log(const char *logfile) {
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

        if (strlen(line)!= 0) {
            unsigned char md5_str[MD5_DIGEST_LENGTH*2];
            char path[MAX_PATH];
            char date[1024];

            int Inner = 0;
            int md5_occurence = 0;
            int path_occurence = 0;
            int date_occurence = 0;


            for (size_t i = 0; i < strlen(line); i++) {

                if (line[i] == ';') {
                    Inner++;
                } else if (Inner == 2) {
                    md5_str[md5_occurence] = line[i];
                    md5_occurence++;
                } else if (Inner == 0) {
                    path[path_occurence] = line[i];
                    path_occurence++;
                } else if (Inner == 1) {
                    date[date_occurence] = line[i];
                    date_occurence++;
                }
            }

            path[path_occurence] = '\0';
            date[date_occurence] = '\0';
            md5_str[md5_occurence] = '\0';

            // on convertie le md5 en `unsigned char`
            unsigned char md5_bytes[MD5_DIGEST_LENGTH];
            md5_hex_to_bytes(md5_str, md5_bytes);


            // Diviser la ligne en 3 parties : chemin, md5 et date

            if (!path[0] || !date[0]) {
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

            for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
                new_element->md5[j] = md5_bytes[j];
            }





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
    }

    fclose(file);

    return log_list;
}






/**
 * @brief Met à jour une ligne dans le fichier `.backup_log`.
 *
 * Cette fonction parcourt les éléments du fichier `.backup_log` et met à jour la ligne correspondant
 * au chemin spécifié dans `element`. Si le MD5 a changé, la ligne est mise à jour. Si aucun élément
 * correspondant n'est trouvé, un nouvel élément est ajouté.
 *
 * @param element L'élément `log_element` contenant les informations à mettre à jour.
 * @param filename Le chemin du fichier `.backup_log` à modifier.
 *
 * @note La fonction ouvre le fichier en mode écriture et le réécrit entièrement.
 */
void update_backup_log(const log_element *elt, const char *filename, const char * dirname) {
    if (!elt || !filename) {
        fprintf(stderr, "Paramètres invalides pour update_backup_log.\n");
        return;
    }

    log_t * backup_log_list = read_backup_log(filename);
    log_element * backup_element = backup_log_list->head;
    FILE* file = fopen(filename, "w");
    fclose(file);
    char * temp1 = reverse_path((char *)filename);
    char * temp2 = short_first_delimiter(temp1);
    char * temp3 = reverse_path(temp2);
    char absolute_elt_path[1024];
    char relative_elt_path[1024];
    snprintf(absolute_elt_path, sizeof(absolute_elt_path), "%s/%s/%s", temp3, dirname, elt->path);
    snprintf(relative_elt_path, sizeof(relative_elt_path), "%s/%s", dirname, elt->path);
    printf("@@%s@@", absolute_elt_path);
    struct stat statbuff;

    printf("%d", stat(absolute_elt_path, &statbuff));
    free(temp1);
    free(temp2);
    free(temp3);
    int changed = 0;
    int found = 0;
    while (backup_element != NULL) {

        char * temp_backup_log_element_path = short_first_delimiter((char *)backup_element->path);


        if (strcmp(temp_backup_log_element_path, elt->path) == 0) {
            found = 1;
            if (memcmp(backup_element->md5, elt->md5, MD5_DIGEST_LENGTH) != 0) {

                strcpy((char *)elt->path, relative_elt_path);
                write_log_element((log_element*)elt, filename);

                if (S_ISREG(statbuff.st_mode)) {

                    backup_file(absolute_elt_path);
                }
                changed = 1;


            } else {

                write_log_element(backup_element, filename);

            }
        } else {

                write_log_element(backup_element, filename);

            }


        free(temp_backup_log_element_path);


        backup_element = backup_element->next;
        continue;

    }
    if (found == 0) {
        strcpy((char *)elt->path, relative_elt_path);
        write_log_element((log_element*)elt, filename);

        if (S_ISREG(statbuff.st_mode)) {

            backup_file(absolute_elt_path);
        }
        changed = 1;
    }
    if (changed == 0) {

        remove(absolute_elt_path);
    }
    free((log_element*)elt);
    free(backup_log_list);






}
/**
 * @brief Écrit un élément de log dans le fichier `.backup_log`.
 *
 * Cette fonction ajoute un nouvel élément de log à la fin du fichier `.backup_log`.
 * Elle écrit le chemin, le MD5 et la date de l'élément sur une nouvelle ligne du fichier.
 *
 * @param elt L'élément de log à écrire (contenant le chemin, le MD5 et la date).
 * @param logfile Le chemin du fichier `.backup_log` où l'élément sera écrit.
 */
void write_log_element(log_element *elt, const char *logfile) {

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
    fprintf(file, "%s;", elt->date);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        fprintf(file, "%02x", elt->md5[i]);
    }
    fprintf(file, "\n");

    fclose(file);
}



/**
 * @brief Crée une nouvelle liste de chemins.
 *
 * Cette fonction alloue et initialise une nouvelle structure de liste `PathList`,
 * avec un tableau dynamique pour stocker les chemins. La capacité initiale du tableau
 * est fixée à 10 éléments.
 *
 * @return PathList * Un pointeur vers la nouvelle liste de chemins, ou NULL en cas d'erreur d'allocation.
 */
PathList *create_pathlist() {
    PathList *list = malloc(sizeof(PathList));
    list->paths = malloc(10 * sizeof(char *)); // Initialisation avec une capacité de 10
    list->count = 0;
    list->capacity = 10;
    return list;
}

/**
 * @brief Ajoute un chemin à la liste dynamique de chemins.
 *
 * Cette fonction ajoute un chemin à la liste `PathList`. Si la liste atteint sa capacité maximale,
 * elle double sa capacité en allouant un plus grand espace mémoire. Le chemin est ajouté à la fin de la liste,
 * et le compteur d'éléments est incrémenté.
 *
 * @param list Pointeur vers la liste de chemins à laquelle ajouter le chemin.
 * @param path Le chemin à ajouter à la liste.
 */
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

/**
 * @brief Libère la mémoire allouée à la liste dynamique de chemins.
 *
 * Cette fonction libère toute la mémoire allouée pour la liste de chemins. Elle parcourt la liste,
 * libère chaque chemin individuel, puis libère le tableau contenant les chemins et enfin la structure
 * représentant la liste.
 *
 * @param list Pointeur vers la liste de chemins à libérer.
 */
void free_pathlist(PathList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->paths[i]); // Libérer chaque chemin
    }
    free(list->paths); // Libérer le tableau de chemins
    free(list); // Libérer la structure
}

/**
 * @brief Parcourt un répertoire et récupère tous les chemins des fichiers et répertoires qu'il contient.
 *
 * Cette fonction ouvre un répertoire, parcourt ses entrées et récupère les chemins des fichiers et des
 * sous-répertoires. Si un sous-répertoire est trouvé, la fonction est appelée récursivement pour l'explorer
 * et récupérer ses fichiers. Les chemins sont ajoutés à une liste dynamique.
 *
 * @param directory Le chemin du répertoire à explorer.
 * @return Une liste de chemins de type `PathList` contenant tous les fichiers et répertoires trouvés.
 * @note La fonction utilise la récursion pour explorer les sous-répertoires. Si le répertoire ne peut pas
 *       être ouvert, la fonction retourne NULL.
 */
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

/**
 * @brief Copie un répertoire et son contenu, y compris les sous-répertoires, vers un autre emplacement.
 *
 * Cette fonction copie récursivement tous les fichiers et sous-répertoires d'un répertoire source vers
 * un répertoire de destination. Si le répertoire de destination n'existe pas, il est créé. Les fichiers et
 * répertoires sont copiés de manière récursive, en parcourant tous les niveaux de sous-dossiers.
 *
 * @param src Le chemin du répertoire source à copier.
 * @param dest Le chemin du répertoire de destination où les fichiers et répertoires seront copiés.
 * @note Si un sous-répertoire est rencontré, il est également copié avec son contenu.
 *       Si un fichier est rencontré, il est copié en écrasant les fichiers existants dans la destination.
 */
void copy_file(const char *src, const char * dest) {
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

        // Vérifier si l'entrée est un fichier ou un répertoire
        struct stat entry_stat;
        if (stat(fullpath, &entry_stat) == -1) {
            perror("Erreur lors de l'appel à stat");
            continue;
        }
        // Si l'entrée est un répertoire, explorer récursivement
        if (S_ISDIR(entry_stat.st_mode)) {
            mkdir(destpath, 0777);  // Créer le dossier
            copy_file(fullpath, destpath); // Recherche les sous dossier et fichiers
        } else{
            // S'il s'agit d'un fichier, on copie le contenue du fichier source dans le fichier destination que
            // l'on crée en meme temps
            FILE *d =fopen(destpath,"w");
            FILE *f =fopen(fullpath,"r");
            int c;
            while ((c = fgetc(f)) != EOF) {
                fputc(c,d);
                }
            fclose(f);
            fclose(d);
	}
}
}
