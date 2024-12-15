Voici la première version du code du projet de LP25
Il est nécessaire de savoir quelques points:

    - Lorsque l'on effectue une backup incrémental celle ci produit une erreur à la fin de type
    free : double free detected, et malgré mes afforts je n'ai tout simplement pas réussis à trouver l'origine 
    de ce problème ni même un moyen de le résoudre à temps.
    Le problème survient juste avant la toute dernière tache qui est de copié collé le backup_log en racine 
    à la source de la backup. Il suffit de le copé à la main pr avoir la backup dans son état voulu

    - Nous avons été pris de court par une erreur au dernier moment lors de la restauration d'une 
    backup. Pour une raison que l'on ignore la fonction undeduplicate_file() produit l'erreur suivante : 
    Erreur d'allocation mémoire pour les chunks: Cannot allocate memory.

    