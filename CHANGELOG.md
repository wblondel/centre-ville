# Journal des versions

La section de chaque version sert de notes à sa release GitHub
(`.github/workflows/build-installer.yml` la reprend jusqu'à la section
suivante, pour la version et ses pré-releases).

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

### Restent à corriger

- **Moteur, trouvé en 6.0.1** : les passagers d'un vehlib n'en descendent
  jamais (`vehlib::FaitDescendrePietons` cherche un trottoir depuis la case
  du vehlib, toujours occupée par lui, et `voie::FaitDescendrePieton` ne
  gère pas les vehlibs). Un vehlib qui a pris un passager repart donc dès
  qu'il arrive.
- **Interface** : la liste « Bogues d'origine conservés » de la 6.0.0 (dans
  `CHANGELOG.md`), et `f_simul.cpp` qui
  teste `if ((hfile=FileCreate(…)))` et `if ((hfile=FileOpen(…)))` alors que
  l'échec vaut -1 : un fichier impossible à créer est signalé comme une
  erreur de données ; un fichier impossible à ouvrir donne une carte vide
  (`centre_ville::Charge` ne voit pas les erreurs de lecture, -1, seulement
  une fin de fichier prématurée).

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

Interface :

- **Fiches d'informations** : à l'arrêt, rien ne les rafraîchit. Bus et
  tram : « Prochain arrêt … depuis le tour » lit le premier bus (le premier
  tram), « Tours effectifs » du tram affiche le tour courant. Piéton dans un
  tram, un taxi ou un vehlib : coordonnées lues dans les bus ; « vers une
  file de vehlibs » testé sur la file de taxis ; libellé « ans un vehlib ».
  Taxi : distance à la tête de file lue avec les indices inversés (valeur
  fausse, lecture hors des tableaux).
- **Distances** : un tableau vide perd sa ligne d'en-tête fixe, même sur une
  autre carte ensuite ; la fiche des files de parkings est plus courte de
  17 pixels (case « Avec couloirs de bus » oubliée dans sa hauteur).
- **Rechercher** : un piéton monté dans un taxi ou un vehlib est situé avec
  les coordonnées des bus (lecture hors du tableau possible), dans un tram il
  est « non utilisé » ; places non définies sans message (« Taxi » et
  « Vehlib » ne testent pas `Defini()`) ; un numéro resté plus grand que le
  nombre d'éléments de la carte reste affiché.
- **Statistiques** : Statistiques véhicules vides jusqu'au tour suivant
  l'ouverture ; le dernier segment d'Évolution, redessinée entre deux tours,
  va vers des statistiques pas encore calculées. Fluidité et trafic : les
  flèches et Origine déplacent le curseur même depuis les champs
  numériques ; Échap ne ferme pas la fiche.
- **Paramètres** : ouvrir une fiche fille (bus, tram, taxis, vehlibs) perd
  les saisies non appliquées.
- **Champs numériques** (`TCSpinEdit`) : Entrée valide une valeur hors des
  bornes (seule la sortie du champ la borne) ; bus et tram : un n° d'arrêt
  trop grand, nul ou vide est remis à 0 dès la frappe.
- **Simulation** : « Arrêt » est grisé en pause ; il faut reprendre, puis
  arrêter.

## [5.4.3.2] — version d'origine

Dernière version C++Builder de Patrice Fouquet, pour Windows (32 et
64 bits). Historique des versions précédentes : `docs/lisezmoi.txt`. Son
code reste dans `src/`, en lecture seule.
