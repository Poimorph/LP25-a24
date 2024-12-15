#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stdio.h>
#include <openssl/md5.h>


/** 
 * @brief Structure représentant un élément d'un fichier de log.
 * 
 * Contient le chemin du fichier, son MD5, la date de modification, et les pointeurs vers
 * les éléments précédent et suivant dans la liste chaînée.
 */
typedef struct log_element {
    const char *path;                       /**Chemin du fichier/dossier */
    unsigned char md5[MD5_DIGEST_LENGTH];   /**MD5 du fichier dédupliqué */
    char *date;                             /**Date de dernière modification */
    struct log_element *next;               /**Pointeur vers l'élément suivant */
    struct log_element *prev;               /**Pointeur vers l'élément précédent */
} log_element;

/**
 * @brief Structure représentant une liste chaînée de logs.
 * 
 * Contient les pointeurs vers le premier (`head`) et dernier (`tail`) élément de la liste.
 */
typedef struct {
    log_element *head; /**Début de la liste de log */
    log_element *tail; /**Fin de la liste de log */
} log_t;

/**
 * @brief Structure représentant une liste dynamique de chemins.
 * 
 * Gère un tableau de chemins, avec un compteur (`count`) et une capacité maximale (`capacity`).
 */
typedef struct {
    char **paths;   /**Tableau de chaînes représentant les chemins */
    int count;      /**< Nombre actuel de chemins dans la liste */
    int capacity;   /**< Capacité actuelle du tableau */
} PathList;

/**
 * @brief Convertit un caractère hexadécimal en valeur entière.
 * 
 * @param c Le caractère hexadécimal à convertir.
 * @return La valeur entière correspondante ou -1 si invalide.
 */
int hex_to_int(unsigned char c);
/**
 * @brief Convertit une chaîne hexadécimale MD5 en tableau de bytes.
 * 
 * @param hex_md5 La chaîne de caractères représentant le MD5 en hexadécimal (32 caractères).
 * @param md5_bytes Le tableau de bytes qui recevra le MD5 converti (16 octets).
 * 
 * @note La fonction vérifie si les caractères hexadécimaux sont valides, et si ce n'est pas le cas,
 *       elle affiche un message d'erreur et retourne sans effectuer la conversion.
 */
void md5_hex_to_bytes(unsigned char * hex_md5, unsigned char * md5_bytes);
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
log_t *read_backup_log(const char *logfile);
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
void update_backup_log(const log_element *element, const char *filename);
/**
 * @brief Écrit un élément de log dans le fichier `.backup_log`.
 * 
 * Cette fonction ajoute un nouvel élément de log à la fin du fichier `.backup_log`. 
 * Elle écrit le chemin, le MD5 et la date de l'élément sur une nouvelle ligne du fichier.
 * 
 * @param elt L'élément de log à écrire (contenant le chemin, le MD5 et la date).
 * @param logfile Le chemin du fichier `.backup_log` où l'élément sera écrit.
 */
void write_log_element(log_element *elt, const char *logfile);
/**
 * @brief Crée une nouvelle liste de chemins.
 * 
 * Cette fonction alloue et initialise une nouvelle structure de liste `PathList`, 
 * avec un tableau dynamique pour stocker les chemins. La capacité initiale du tableau 
 * est fixée à 10 éléments.
 * 
 * @return PathList* Un pointeur vers la nouvelle liste de chemins, ou NULL en cas d'erreur d'allocation.
 */
PathList *create_pathlist();
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
void add_path(PathList *list, const char *path);
/**
 * @brief Libère la mémoire allouée à la liste dynamique de chemins.
 * 
 * Cette fonction libère toute la mémoire allouée pour la liste de chemins. Elle parcourt la liste,
 * libère chaque chemin individuel, puis libère le tableau contenant les chemins et enfin la structure
 * représentant la liste.
 * 
 * @param list Pointeur vers la liste de chemins à libérer.
 */
void free_pathlist(PathList *list);
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
PathList *list_files(const char *directory);
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
void copy_file(const char *src, const char *dest);

#endif // FILE_HANDLER_H