# Journal des versions

La section de chaque version sert de notes à sa release GitHub
(`.github/workflows/build-installer.yml` la reprend jusqu'à la section
suivante, pour la version et ses pré-releases).

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

Moteur (`engine/`, identique à l'original) :

- **Blocage des vehlibs** : `vehlib::ReservePlaceVehlibCible` tire au hasard
  une place libre sans limite d'essais. Quand plus de vehlibs reçoivent un
  passager qu'il ne reste de places libres, la simulation se fige. Sur
  Géant (29 vehlibs pour 32 places), cela dépend du tirage : par exemple
  avant le tour 50 avec la graine 123.
- `Verifie*()` reçoivent `asMsgErr` par valeur : leurs messages sont perdus.
- `PrepareCopie(FIN, …)` (« Copier vers… ») remet à zéro `NbY` au lieu de
  `NbYc`.
- `voie::Sauve` contient un `;` au lieu de `&&` : le résultat des dernières
  écritures est ignoré.
- `depot_bus::Verifie` appelle `Format("Dépot bus : ", …)` sans
  spécificateur.
- Champs non initialisés : `centre_ville::NbVehlibs` (fichiers antérieurs
  au format 0x530, enregistré tel quel), `pieton::TourAttBus`,
  `TourAttTram`, `TourAttTaxi`, `centre_ville::FeuP` sans feux piétons (le
  portage remet la mémoire à zéro).

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
