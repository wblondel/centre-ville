# Journal des versions

La section de chaque version sert de notes à sa release GitHub
(`.github/workflows/build-installer.yml` la reprend jusqu'à la section
suivante, pour la version et ses pré-releases).

## [6.0.12] — fiche Rechercher et carte, trois bogues corrigés

Cette version corrige trois bogues trouvés en testant les chemins de la carte
(`grille/GrilleSimulation.cpp`) et de la fiche Rechercher que les tests ne parcouraient pas :
le premier vient de l'original, les deux autres du port. Le format des fichiers `.cvw` ne
change pas (0x530), ni la simulation des cartes livrées (golden master identique).

### Bogues corrigés

- **Rechercher déplaçait le curseur à son ouverture** : sur une case qui porte le numéro d'un
  élément posé ailleurs (arrière d'un bus ou d'un tram, autre case d'un arrêt de bus ou de
  tram, place de taxi ou de parking dont la file commence ailleurs), la fiche plaçait le
  curseur sur l'avant du véhicule, la case de l'arrêt ou la queue de la file. Les
  coordonnées affichées restaient pourtant celles de la case. Le curseur reste maintenant sur
  la case, sauf si le bouton de cet élément est coché : la fiche cherche alors l'élément,
  comme avant.
- **Sélection glissée au-delà de la carte visible** : la carte défilait de plusieurs cases
  toutes les 60 ms, d'autant plus que la souris était loin, et la sélection sautait jusqu'à
  la case sous la souris. Un mouvement à gauche ou au-dessus de la carte la faisait aussi
  défiler. Comme dans la version d'origine, la sélection ne s'étend plus que d'une case
  au-delà des cases visibles, puis d'une case toutes les 60 ms.
- **Menu contextuel ouvert au clavier** (touche Menu ou Maj+F10, sous Windows et Linux) : il
  s'ouvrait hors de la carte quand la case courante n'était pas visible. Il s'ouvre alors en
  haut à gauche de la carte, comme dans la version d'origine.

### Tests

Nouveaux tests de l'interface :

- l'ouverture de Rechercher sur chacune de ces cases, avec le bouton Coordonnées puis celui
  de l'élément coché, et sur la case de l'élément ;
- le message de chaque sorte d'élément de Rechercher : numéro au-delà du nombre d'éléments,
  élément non utilisé ;
- la carte au clavier (flèches, Maj, Ctrl, Origine, Fin, pages), à la molette et aux barres
  de défilement, dans une fenêtre plus petite que la carte ;
- la sélection glissée au-delà de la vue, et le menu contextuel à la souris et au clavier.

## [6.0.11] — fiches Priorités après une commande sur la sélection

Cette version corrige un bogue de l'original repéré en testant la fiche Carrefours. Le format
des fichiers `.cvw` ne change pas (0x530), ni la simulation des cartes livrées (golden master
identique).

### Bogue corrigé

- **Fiche Priorités restée ouverte pendant une commande sur la sélection** : après
  Carrefours, Effacer les routes ou les voies de tram, un segment, un coin ou un
  périphérique, la fiche Priorités (ou Priorités piétons) montrait encore la case d'avant.
  OK y réécrivait alors les anciennes priorités et les anciens feux. Un feu pouvait ainsi
  être reposé sur une case effacée, ou ajouté à l'entrée d'un carrefour sur une direction
  qu'elle n'a plus. La fiche est maintenant réaffichée après la commande, comme après
  l'édition d'une case : elle montre la case modifiée, ou laisse la place à l'autre fiche
  Priorités si la case a gagné ou perdu ses voies. La fiche Feu, si elle était ouverte, se
  ferme sans rien écrire, comme après un clic sur la carte.

### Tests

Nouveau test de l'interface : chacune de ces commandes sur une case dont une fiche Priorités
est ouverte, puis OK, qui ne doit pas modifier la case. Les deux tests de la 6.0.10 où la
fiche Feu est ouverte pendant un carrefour vérifient maintenant qu'elle se ferme.

## [6.0.10] — fiche Carrefours, trois bogues corrigés

Cette version corrige trois bogues trouvés en testant les chemins de la fiche Carrefours que
les tests ne parcouraient pas. Le format des fichiers `.cvw` ne change pas (0x530), ni la
simulation des cartes livrées (golden master identique).

### Bogues corrigés

- **Sélection d'une autre taille** : la fiche activait aussi les boutons d'une taille qui
  donnait le même nombre « 10 × colonnes + lignes ». Une sélection de 2 colonnes sur 14
  lignes proposait ainsi les carrefours de 3 sur 4 (de même pour 3 × 13, 3 × 14, 5 × 16 et
  7 × 18). Le carrefour était alors lu hors de sa table, d'où des directions quelconques.
  Ses priorités étaient posées dans la colonne à droite de la sélection, et hors de la
  mémoire du réseau quand celle-ci touchait le bord droit de la carte. Seuls les boutons
  de la taille exacte de la sélection sont maintenant actifs.
- **Fiche Feu ouverte pendant l'ajout de feux** : cette fiche, qui n'est pas modale,
  gardait l'adresse de son feu. Quand un carrefour à feux, ou la fiche Priorités,
  agrandissait la table des feux, OK et Appliquer écrivaient dans l'ancienne table, déjà
  libérée. Le feu n'était donc pas modifié, et la mémoire pouvait être corrompue.
- **Fiche Feu ouverte sur un feu effacé par un carrefour** : le carrefour reprend le
  numéro du feu effacé pour l'un de ses feux. OK ou Appliquer y écrivait alors les valeurs
  de la fiche, et ce feu du carrefour n'attendait plus que les autres passent au rouge. La
  fiche se ferme maintenant sans rien écrire.

### Tests

Nouveaux tests de l'interface :

- chaque bouton de la fiche (taille, orientation, avec ou sans feux). Le dessin est
  vérifié par sa structure : aucune impasse, chaque entrée mène à chaque sortie, priorités
  et feux aux seules entrées, états et listes des feux ;
- sélections d'une autre taille, et l'aide ;
- table des feux agrandie de ce qui manque ;
- fiche Feu ouverte pendant l'ajout ou l'effacement de feux.

Le test existant de la fiche cliquait sur son bouton Annuler. Ce bouton est invisible
(0 × 0 pixel), le clic restait sans effet et la fiche ne se fermait qu'au bout des
10 secondes de garde du test. Le test ferme maintenant la fiche par Échap.

Le tri des lignes encore non exécutées est dans `app/README.md`, « Couverture de
`f_carfours.cpp` ».

## [6.0.9] — fiches des lignes de bus et de tram, un bogue corrigé

Cette version corrige un bogue trouvé en testant les chemins des fiches Paramètres des
lignes de bus et de tram que les tests ne parcouraient pas. Le format des fichiers `.cvw`
ne change pas (0x530), ni la simulation des cartes livrées (golden master identique).

### Bogue corrigé

- **Ligne qui passe plusieurs fois par le même arrêt** : la ligne était ajoutée à la
  liste des lignes de l'arrêt à chaque passage (« Arrêt bus 25 (3,4,4) » dans la barre
  d'état). Au dixième passage par un même arrêt, toutes lignes confondues, la liste
  débordait sur l'arrêt suivant, qui pouvait changer de place, ou hors de la mémoire
  réservée aux arrêts pour le dernier. Chaque ligne n'est maintenant listée qu'une fois.

### Tests

Nouveaux tests de l'interface pour les deux fiches : Appliquer, arrêts non définis
refusés, réseau dont tous les arrêts ont été retirés, flèches de déplacement du curseur
(avec « Coins »), clignotement de l'arrêt choisi, aide, case « Tout » pendant la
simulation. Le tri des lignes encore non exécutées est dans `app/README.md`, « Couverture
de `f_prmbus.cpp` et `f_prmtram.cpp` ».

## [6.0.8] — couverture de la fiche principale, deux bogues corrigés

Cette version corrige deux bogues trouvés en testant les chemins de la fiche principale que
les tests ne parcouraient pas, et retire le code que rien n'appelait. Le format des fichiers
`.cvw` ne change pas (0x530), ni la simulation des cartes livrées (golden master identique).

### Bogues corrigés

- **Affichage épidémie sans piéton** : sur un réseau sans piéton, la barre d'état divisait
  par le nombre de piétons à chaque tour, ce qui plantait l'application sous Windows et
  Linux. Elle donne maintenant les statistiques des véhicules.
- **Place de parking ou de vehlib inaccessible** : la vérification des distances oubliait
  ces places (depuis les versions 5.0 et 5.3). La simulation démarrait, et les véhicules
  destinés à une place qu'on ne peut pas rejoindre tournaient sans fin : elle ne se
  terminait jamais. Marche signale maintenant le réseau non connexe et affiche le tableau
  des distances, comme pour les arrêts et les files de taxis.
- **Réseau non connexe pour une file de taxis** : le message parlait d'une file de
  parkings.

### Tests

Nouveaux tests de l'interface : chaque message de la vérification du réseau, comparé à sa
copie dans le harnais des tests du moteur ; coins, périphériques et segments de deux voies ;
voies et arrêts de tram ; directions et double-clic ; commandes d'affichage et nombre de
tours des statistiques ; dessins pendant la simulation (piétons écrasés et morts, trajet des
lignes, « Suivre… ») ; confirmations de l'effacement d'une sélection ; arrêts, files et
places inaccessibles. Le tri des lignes encore non exécutées est dans `app/README.md`,
« Couverture de `f_simul.cpp` ».

### Maintenance

- Code sans appelant retiré du moteur et de l'application : méthode et liste dans
  `engine/README.md`, « Code mort retiré ».
- Le test de l'aide ne lit plus `src/`.

## [6.0.7] — place de vehlib supprimée, couverture du moteur

Cette version corrige un plantage de l'original, trouvé grâce au rapport de couverture des
tests. Le format des fichiers `.cvw` ne change pas (0x530), ni la simulation (golden master
identique).

### Bogues corrigés

- **Place de vehlib supprimée** : Marche lançait la simulation, qui lisait hors du réseau
  (plantage). Comme pour une place de parking, la vérification du réseau signale maintenant
  la place manquante : enregistrer puis rouvrir le réseau renumérote les places.
- **Contrôle d'intégrité** : les messages d'une durée par défaut du vert ou du rouge
  négative donnaient la durée de l'orange.

### Tests

Nouveaux tests du moteur sur des chemins que les cartes livrées ne parcouraient pas : places
de parking et de vehlib supprimées puis renumérotées à la réouverture, arrêts de bus et de
tram retirés case par case, arrêts et feux de piétons ajoutés quand tous les numéros sont
pris, épidémie avec réinfections. Le tri des lignes encore non
exécutées est dans `engine/README.md`, « Couverture des tests ».

## [6.0.6] — deux accès hors tableau corrigés, linter

Cette version corrige deux accès hors tableau de l'original, trouvés par le nouveau linter et
confirmés par AddressSanitizer. Le format des fichiers `.cvw` ne change pas (0x530), ni la
simulation (golden master identique).

### Bogues corrigés

- **Case avec trois ou quatre feux** : pendant le choix d'une liste de feux, le double-clic
  écrivait hors d'un tableau de deux éléments (plantage possible). Le double-clic parcourt
  maintenant les deux premiers feux de la case : les suivants ne peuvent pas être choisis.
- **Effacer une sélection contenant un arrêt de bus ou de tram** : lecture hors du tableau
  pour le dernier arrêt.

### Outillage

`clang-tidy` pour le C++, `ruff` pour le Python, `actionlint` et `shellcheck` pour les workflows,
dans la CI.
Voir `README.md`, « Lint », et `PLAN_CI.md`.

## [6.0.5] — feux et zone générée, lectures et écritures hors des tableaux corrigées

Cette version corrige les listes de feux, la suppression d'un feu que d'autres feux utilisent
et la génération d'une zone plus petite, trouvées par de nouveaux tests des fiches Feux. Ces
bogues venaient de l'original. Le format des fichiers
`.cvw` ne change pas (0x530), ni la simulation (golden master identique).

### Bogues corrigés

- **Listes de feux** (demande, attente, feux de piétons) : retirer le dernier feu d'une liste
  écrivait hors du tableau (corruption de la mémoire, plantage possible plus tard).
- **Nouveau, zone générée plus petite** : pendant la génération, la grille était redessinée
  aux anciennes dimensions, hors du réseau (plantage possible).
- **Message de suppression d'un feu** : il annonçait le feu suivant celui qui référence, et
  lisait hors du tableau quand c'était le dernier (par exemple « feu n°4 (0,0) » pour le
  feu n°3 en (8,4)). Une référence par un feu de piétons donnait aussi des coordonnées
  fausses.
- **Feu supprimé, référencé par un feu de piétons** : la référence restait.
- **Effacer les routes d'une sélection** : un feu sur une route à sens unique était effacé
  sans question, et les feux qui le référençaient gardaient cette référence. Les feux de
  piétons n'étaient pas traités non plus.

Une carte enregistrée avant cette version peut encore contenir ces références : elles ne
sont pas réparées à l'ouverture.

### Tests

Nouveaux tests par l'interface : fiche Feux (Appliquer, Annuler, listes de feux par
double-clic, réouverture), suppression d'un feu référencé par un feu ou un feu de piétons,
effacement d'une sélection.

## [6.0.4] — modifications qui pouvaient se perdre, édition de carte testée

Cette version corrige deux pertes de modifications, trouvées par de nouveaux tests du
parcours d'édition d'une carte. Ces bogues venaient de l'original. Le format des fichiers
`.cvw` ne change pas (0x530), ni la simulation (golden master identique).

### Bogues corrigés

- **Copier vers…** : après la copie, le réseau entier prenait le nom de la copie et n'était
  plus marqué modifié. « Enregistrer » écrasait alors la copie par tout le réseau, et quitter
  perdait les modifications sans poser de question. Le réseau garde maintenant son nom, son
  titre et ses modifications.
- **Dépôt de bus** : le poser ou le retirer ne marquait pas la carte comme modifiée. Quitter
  ou ouvrir une autre carte le perdait sans poser de question.
- **Tracer une route** : deux coordonnées étaient lues sans valeur pour un segment d'une
  seule voie. Sans effet visible, trouvé en relisant le code.

### Tests

Nouveaux parcours par l'interface : création d'un réseau jusqu'à la fin de sa simulation,
parkings, feu déplacé, Enregistrer sous et réouverture, chaque édition marquant la carte
modifiée, Quitter avec des modifications, Copier vers, effacement d'une sélection.

## [6.0.3] — plantages évités, fiche Piéton et champs numériques corrigés

Cette version corrige deux causes de plantage trouvées en relisant le code, et trois
comportements de l'interface que la 6.0.2 avait gardés. Le format des fichiers `.cvw` ne
change pas (0x530), ni la simulation (golden master identique).

### Plantages

- **Moins de deux parkings** (trouvé en 6.0.3) : la vérification du réseau et l'ouverture
  des Paramètres divisaient par zéro, ce qui arrête l'application sur un processeur x86
  (Windows, Linux, Mac Intel). La vérification signale maintenant le nombre de parkings, et
  les Paramètres s'ouvrent.
- **Dimensions de la zone** (trouvé en 6.0.3) : aucune limite haute. Une zone de
  99999x99999 demandait 2,5 To de mémoire. Une nouvelle zone est limitée à 200x200, ce qui
  laisse assez de pile au calcul récursif des distances. Une carte plus grande s'ouvre
  toujours.

### Fiche Piéton

- Quand Rechercher suit un piéton, la fiche montre celui de la case du curseur, et non plus
  le piéton suivi. Sur une case sans piéton, elle garde le piéton suivi.

### Champs numériques

- **Un seul numéro possible** (par exemple Fluidité et trafic sur une carte à un seul
  parking) : un numéro tapé est ramené à ce numéro. Seuls les champs sans limites (minimum
  et maximum à 0) ne sont pas bornés.
- **Texte vide, « - » ou « + »** : remplacé par la valeur minimale en quittant le champ.

Détail : `app/README.md`, « Corrections de la 6.0.3 », et `engine/README.md`.

## [6.0.2] — bogues d'origine de l'interface corrigés, fichiers plus sûrs

Cette version corrige les bogues de l'interface que les versions précédentes
avaient conservés, et rend l'ouverture et l'enregistrement plus sûrs. Le format
des fichiers `.cvw` ne change pas (0x530) : un fichier enregistré par la 6.0.2
s'ouvre toujours dans la 5.4.3.2, et inversement. La simulation ne change pas
(golden master identique).

### Lectures hors des tableaux

- **Taxi** : la distance à la tête de file était lue avec les indices inversés
  (valeur fausse, lecture hors du tableau).
- **Rechercher et fiche Piéton** : un piéton dans un taxi ou un vehlib (et, pour la
  fiche, dans un tram) était situé avec les coordonnées des bus.
- **Fiche Piéton** (trouvé en 6.0.2) : la tête de la file de taxis était lue une
  place plus loin, hors du tableau pour la dernière place.
- **Clic sur la carte après Arrêt** (trouvé en 6.0.2) : avec Rechercher ouverte sur
  un piéton et la fiche Piéton ouverte, accès à un pointeur nul.
- **Vérification du réseau** : borne testée après l'accès (`PietonPeutAller`).

### Fiches d'informations

- **À l'arrêt**, elles montrent aussitôt le réseau rechargé, au lieu du dernier objet
  jusqu'au clic suivant ; de même après Ouvrir et Nouveau.
- **Bus et tram** : « Prochain arrêt … depuis le tour » est celui du véhicule affiché,
  et non du premier.
- **Piéton** : « Se dirige vers une file de vehlibs » n'est plus affiché pour une file
  de taxis ; libellé « Dans un vehlib » (« ans un vehlib »).

### Rechercher

- Un piéton dans un tram est trouvé (il était « non utilisé »).
- Taxi, vehlib et place sans position : « non utilisé(e) », au lieu d'envoyer le curseur
  en (-1, -1) ou de garder le texte précédent.
- Un numéro plus grand que le nombre d'éléments de la carte est ramené dans les bornes à
  l'ouverture, sans déplacer le curseur.

### Distances, statistiques, paramètres

- **Distances** : la ligne d'en-tête fixe revient après un tableau vide ; la fiche des
  files de parkings a sa hauteur complète (17 pixels de plus).
- **Statistiques véhicules** : remplies dès l'ouverture.
- **Évolution** : redessinée entre deux tours, la courbe s'arrête au dernier tour joué.
- **Fluidité et trafic** : dans les champs numériques, les flèches, Origine, Fin et les
  pages restent au champ ; Échap ferme la fiche.
- **Paramètres** : les saisies non appliquées sont gardées après une fiche fille.

### Champs numériques et simulation

- **Entrée** ramène la valeur dans les bornes avant de valider la fiche.
- **Bus et tram** : le n° d'arrêt n'est plus remis à 0 pendant la frappe, mais en quittant
  le champ ou à Entrée. Un numéro trop grand devient le dernier arrêt, comme dans les
  autres champs.
- **« Arrêt »** est disponible pendant la pause, et le bilan de cet arrêt a la même durée
  que celui de la pause.

### Fichiers

- **Enregistrer** écrit dans un fichier temporaire voisin, puis remplace l'original d'un
  seul geste : un disque plein ne détruit plus le fichier. Un fichier en lecture seule
  reste refusé. Dans un dossier en lecture seule, un fichier inscriptible est encore écrit
  directement : c'est le seul cas où une écriture interrompue peut le tronquer.
- **Ouvrir** : le réseau affiché n'est remplacé qu'une fois le fichier chargé. Un fichier
  d'une version ultérieure, illisible, coupé ou incohérent affiche un message et garde le
  réseau, sans plantage ni fermeture de l'application.
- **Moteur** : `centre_ville::Charge` voit les erreurs de lecture et les champs lus en
  partie, et refuse un nombre d'éléments négatif ou plus grand que le fichier, ainsi qu'une
  zone sans case ; `~centre_ville` libère `FeuP`, `Bus` et `Tram`.

### Restent

- **« Tours effectifs » du tram** n'a pas changé : signalé comme bogue en 6.0.0, il est
  correct, car tous les trams partent au tour 0.
- **Paramètres des bus et des trams** : les flèches y déplacent toujours le curseur de la
  carte depuis les champs numériques, ce qui sert à montrer un arrêt.
- **Lien physique** (hard link) vers un fichier enregistré : il garde l'ancien contenu.

Détail : `app/README.md`, « Corrections de la 6.0.2 », et `engine/README.md`.

## [6.0.1] — bogues d'origine du moteur corrigés

Première version corrective : la règle du portage (« on porte, on ne corrige
pas ») est levée. Cette version corrige les bogues du moteur de l'original
que la 6.0.0 avait conservés ; ceux de l'interface viendront ensuite. Le
format des fichiers `.cvw` ne change pas (0x530) : un fichier enregistré par
la 6.0.1 s'ouvre toujours dans la 5.4.3.2, et inversement.

### Bogues corrigés

- **Blocage des vehlibs** : quand aucune place de vehlib n'était libre, la
  simulation se figeait (sur Géant, 29 vehlibs pour 32 places, selon le
  tirage : 10 graines sur 60 avant le tour 2000). Un vehlib ne part plus sans
  place d'arrivée réservée : faute de place libre, il attend à la sienne et
  réessaie au tour suivant, et le piéton qui l'attend reste dehors. Quand une
  place est libre, la simulation est celle de l'original.
- **Passagers des vehlibs** (trouvé en 6.0.1) : ils n'en descendaient
  jamais. Un vehlib qui avait pris un passager repartait dès son arrivée, ce
  qui usait les places libres, et le piéton n'était plus jamais simulé
  (`vehlib::FaitDescendrePietons` cherchait un trottoir depuis la case du
  vehlib, toujours occupée par lui, et `voie::FaitDescendrePieton` ne gérait
  pas les vehlibs). À l'arrivée, les passagers descendent désormais sur un
  trottoir voisin de la place, comme ceux d'un taxi, et redeviennent des
  piétons ; le vehlib attend à sa place d'arrivée, devenue sa place de
  départ, qu'un piéton le prenne. Si aucun trottoir voisin n'a de place, le
  passager reste à bord et descendra à la place suivante, comme l'original
  le prévoyait. La simulation de Géant, seule carte livrée avec des vehlibs,
  change.
- **Erreurs d'enregistrement** : une erreur d'écriture (disque plein, fichier
  en lecture seule, fichier impossible à créer) affiche « Erreur de données
  dans le fichier » et « Enregistrement NON effectué. » au lieu d'annoncer
  « Enregistrement effectué. » (`voie::Sauve` contenait un `;` au lieu de
  `&&`, et le résultat des écritures n'était jamais vérifié).
- **Vérification d'intégrité** : les contrôles de position, de direction, de
  priorité, de directions possibles, d'état de feu et de type de demande du
  vert ou de passage au rouge rendent leurs messages (`Verifie*()` les
  recevaient par valeur et les perdaient). Les feux supprimés, qui laissent
  une place libre dans la liste des feux, ne sont pas signalés. Le message
  d'erreur du dépôt de bus donne ses coordonnées.
- **Copier vers…** : un élément non défini (dépôt de bus absent, feu
  supprimé, place de taxi non placée…) garde ses coordonnées -1 dans la copie,
  au lieu de -1 moins le coin de la sélection (-2 pour une sélection en
  (1,1)), que la vérification d'intégrité signalerait à la réouverture ; la
  fin de la copie remet bien toute la sélection à zéro (`PrepareCopie(FIN, …)`
  remettait `NbY` au lieu de `NbYc`).
- **Champs non initialisés** : le moteur initialise lui-même le nombre de
  vehlibs (fichiers antérieurs au format 0x530), la liste des feux de
  piétons, les durées par défaut du vert et du rouge d'un nouveau réseau, et
  les tours d'attente d'un piéton. Il ne dépend plus de la remise à zéro de la
  mémoire faite par l'application et les tests.
- **Lectures hors tableau** (même motif que `voie::DefinitNature`, corrigé en
  6.0.0 : borne testée après l'accès) : recherche de l'arrêt suivant d'un bus
  ou d'un tram, ajout et retrait d'un piéton sur une case, descente des
  passagers d'un vehlib. Aucune ne se produisait sur les cartes livrées.

Détail : `engine/README.md`, « Corrections de la 6.0.1 ».

### Démarrage depuis un fichier non inscriptible (trouvé en recette de la 6.0.1-rc1)

Marche enregistre le réseau avant de démarrer, mais après l'avoir préparé
(l'enregistrement écrit ce que préparent les `Prepare*`). Sur un fichier en
lecture seule, comme les cartes du dossier `Cartes` du `.dmg`, Marche affichait
donc « Erreur de données dans le fichier » et ne démarrait pas ; le réseau restait
préparé, et le clic suivant tournait sans fin dans `centre_ville::PrepareBus`.

- **Fichier en lecture seule, réseau non modifié** : la simulation démarre, sans
  enregistrer ni message, avec les mêmes tirages qu'avec un fichier inscriptible.
  Arrêt recharge le réseau d'origine.
- **Fichier en lecture seule, réseau modifié** : Marche le dit et propose
  d'enregistrer sous un autre nom. Sinon la simulation ne démarre pas et les
  modifications sont gardées.
- **Démarrage interrompu** (erreurs d'intégrité, trams impossibles à placer,
  enregistrement annulé ou en échec, mémoire insuffisante) : le réseau est remis
  dans son état d'avant le clic, modifications non enregistrées comprises, et
  Arrêt reste grisé. Marche prend avant de préparer une copie du réseau dans le
  dossier temporaire ; si elle est impossible, Marche le dit et ne démarre pas.
- **Réseau non connexe** : les tableaux de distances restent affichés pour
  trouver la case fautive, mais le réparer puis relancer Marche libérait ces
  distances d'après le nouveau nombre de directions, hors des tableaux (erreur
  mémoire de l'original). Modifier le réseau les libère désormais et vide les
  tableaux affichés.
- **Fichier inaccessible à l'arrêt** (volume éjecté, fichier supprimé) : le
  réseau d'avant la simulation est rétabli, à enregistrer ailleurs.
- **Réseau sans nom** : Arrêt recharge le fichier choisi au démarrage, sans
  demander lequel ouvrir (annuler cette boîte laissait le réseau préparé).
- **Trams impossibles à placer** : un seul message, sans le « Mémoire
  insuffisante ! » qui le suivait.
- **Fichier impossible à créer ou à ouvrir** (`FileCreate`, `FileOpen` : l'échec
  vaut -1) : « Impossible de créer le fichier » au lieu d'« Erreur de données » ;
  « Impossible d'ouvrir le fichier » au lieu d'une carte vide, le réseau affiché
  étant gardé. Après un échec, Enregistrer vise toujours le fichier courant.
- **Enregistrement interrompu** (disque plein) : le réseau reste signalé comme
  modifié, car le fichier est tronqué.
- **macOS** : au premier lancement, les 6 cartes d'exemple, embarquées dans le
  paquet, sont copiées dans `~/Library/Application Support/Patquoi.fr/Centre-Ville`,
  sans écraser un fichier existant ni recopier une carte supprimée ensuite, comme
  l'AppImage et l'installeur Windows. Le dossier `Cartes` du `.dmg` reste.

### Restent à corriger

- **Interface** : la liste « Bogues d'origine conservés » de la 6.0.0 (dans
  `CHANGELOG.md`).
- **Moteur** : `centre_ville::Charge` ne voit pas les erreurs de lecture (-1)
  d'un fichier ouvert, seulement une fin de fichier prématurée.

## [6.0.0] — portage Qt 6

Centre-Ville 6.0.0 est la version 5.4.3.2 de Patrice Fouquet
([patquoi.fr](http://patquoi.fr#cvw)), portée de C++Builder (VCL, Windows
seulement) vers **C++20, CMake et Qt 6**, et désormais native sous Windows,
macOS et Linux. Le portage garde le même moteur de simulation (recompilé,
vérifié par un « golden master »), les mêmes fenêtres, menus, barres
d'outils et raccourcis, la même aide et le même format de fichier `.cvw`
(version 0x530). Sa règle : on porte, on ne réécrit pas. Les bogues de
l'original sont donc conservés (liste plus bas) et seront corrigés après la
bascule.

### Plateformes et paquets

| Système | Paquet | Contenu |
|---|---|---|
| Windows 10/11, 64 bits (ARM64 par l'émulation x64) | `Centre-Ville-<version>-Windows-x64-Setup.exe` | Installeur Inno Setup : même identifiant que l'installeur 5.4.3.2, qu'il met à jour ; association des fichiers `.cvw` ; aide ; cartes d'exemple dans `%LOCALAPPDATA%\Patquoi.fr\Centre-Ville`. |
| macOS 12 ou plus récent (Apple Silicon et Intel) | `Centre-Ville-<version>-macOS.dmg` | `Centre-Ville.app` à glisser dans Applications, association des fichiers `.cvw`, dossier `Cartes`. |
| Linux x86_64, glibc 2.35 ou plus récente (Ubuntu 22.04, Debian 12…) | `Centre-Ville-<version>-x86_64.AppImage` | Application et aide ; copie les cartes d'exemple dans `~/.local/share/Patquoi.fr/Centre-Ville` au premier lancement. |
| Sources | `Centre-Ville-<version>-sources.tar.gz`, `.zip` | Code du portage et code VCL d'origine (`src/`). |

### Fin de la version 32 bits

Qt 6 n'existe qu'en 64 bits : `cvw.32.exe` n'est plus fourni. Le format des
fichiers `.cvw` ne dépend pas de l'architecture (champs de taille fixe) :
ceux d'une installation 32 bits s'ouvrent comme les autres.

### Limites connues

- **Paquets non signés** : SmartScreen (Windows) et Gatekeeper (macOS)
  avertissent au premier lancement ; voir le README pour les ouvrir quand
  même.
- **Polices** : MS Sans Serif et Small Fonts, polices « raster » que Qt ne
  dessine pas, sont remplacées par Microsoft Sans Serif (aux mêmes mesures,
  sous Windows et macOS) et par Verdana ou Tahoma ; sous Linux, par la
  police du système. Quelques libellés restent coupés (Fluidité et trafic,
  exposants de Composition). Les boutons et les cadres suivent le style du
  système (sous Windows, celui de Windows 10, « windowsvista », dont les
  champs numériques sont lisibles), et le thème sombre de macOS.
- **macOS** : menus dans la barre du système ; « Ctrl » des raccourcis
  devient ⌘, sauf ⌘Q et ⌘H, réservés au système : Carrefours et Rechercher
  se font avec Control+Q et Control+H ; Ctrl+flèches et Ctrl+Espace se font
  avec ⌘. Boîtes de message natives, sans titre.
- **Aide** : F1 ouvre la rubrique dans le navigateur par une petite page de
  redirection (dossier temporaire), pour que le navigateur arrive à la
  bonne rubrique. Sous Linux, un navigateur confiné (snap ou Flatpak, comme
  le Firefox d'Ubuntu) ne lit pas l'aide de l'AppImage.
- **Écarts volontaires** (là où l'original plantait ou lisait hors d'un
  tableau) : « Évolution » en mode épidémique sans piéton ne trace rien
  (division par zéro dans l'original) ; « Zoom (-/+) » de Fluidité et trafic
  est sans effet (violation d'accès) ; les divisions par zéro des fiches
  d'informations affichent NAN ou INF ; un numéro tapé hors des bornes ne
  fait plus lire hors des tableaux.
- **Champs numériques** : la molette et Page précédente / Page suivante
  changent la valeur (la VCL n'y réagissait pas).
- **Vitesse** : même période de simulation, mais la vitesse maximale perçue
  dépend de la résolution des minuteries du système.

Détail : `app/README.md`, « Écarts connus avec l'original ».

### Bogues d'origine conservés, à corriger après la bascule

Moteur : corrigés en 6.0.1 (voir plus haut).

Interface : corrigés en 6.0.2 (voir plus haut).

## [5.4.3.2] — version d'origine

Dernière version C++Builder de Patrice Fouquet, pour Windows (32 et
64 bits). Historique des versions précédentes : `docs/lisezmoi.txt`. Son
code reste dans `src/`, en lecture seule.
