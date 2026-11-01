# Recette d'une version de Centre-Ville

À faire au besoin sur les paquets d'une version, sur de vrais systèmes : une
pré-release n'est pas obligatoire. Les tests automatiques (`ctest`, CI) couvrent
le moteur, les fiches et l'aide en mode « offscreen », mais pas
l'installation, le système de fichiers, ni les navigateurs. Noter chaque
écart (système, version, étapes, capture) dans une issue GitHub.

## Windows 10 ou 11 (64 bits)

Paquet : `Centre-Ville-<version>-Windows-x64-Setup.exe`.

Sans machine Windows : le workflow « Recette Windows »
(`.github/workflows/recette-windows.yml`, lancé avant chaque release, ou à la main sur un tag) refait
sur une machine Windows de GitHub l'installation, l'ouverture d'un `.cvw` par
son association, F1, « À propos » et la désinstallation, sur une machine
neuve et en mise à jour d'une 5.4.3.2 installée par l'installeur d'origine
(`http://patquoi.free.fr/INSCVW95.EXE`) ; il vérifie que l'original ouvre
des fichiers enregistrés par la 6.0.0, et joint des captures d'écran au
run, dont chaque fiche dans l'affichage natif de Windows. Il ne couvre ni
SmartScreen, ni Windows 10/11 grand public, ni ARM64.

Installation neuve (machine sans Centre-Ville) :

- [ ] SmartScreen : « Informations complémentaires », puis « Exécuter quand même ».
- [ ] L'assistant est en français ; licence et lisez-moi (avec l'entrée [Version 6.0.0]) s'affichent.
- [ ] Installation dans `C:\Program Files\Patquoi.fr\Centre-Ville` : `bin\cvw.exe`, `bin\html`, `bin\tutocv`, `plugins`, `translations`.
- [ ] Cartes d'exemple dans `%LOCALAPPDATA%\Patquoi.fr\Centre-Ville` (6 fichiers `.cvw`).
- [ ] Menu Démarrer › Patquoi.fr › Centre-Ville : application, Aide, Lisez-moi, Sources, Désinstaller.
- [ ] « Ouvrir Centre-Ville ! » en fin d'installation : écran « À propos » animé (« 6,0 » « ,0 », « version 64 bits »), puis fenêtre principale.
- [ ] Panneau « Applications installées » : Centre-Ville 6.0.0.

Mise à jour d'une 5.4.3.2 (installeur d'origine de patquoi.fr, 64 ou 32 bits) :

- [ ] L'installeur 6.0.0 reconnaît l'installation existante (même dossier proposé) et la met à jour.
- [ ] Une seule entrée « Centre-Ville » dans les applications installées, en 6.0.0.
- [ ] Cartes d'exemple de `%LOCALAPPDATA%\Patquoi.fr\Centre-Ville` réécrites, comme avec l'installeur d'origine (le lisez-moi conseille de renommer les exemples modifiés) ; les autres fichiers de l'utilisateur sont gardés.
- [ ] `cvw.32.exe` éventuel : plus utilisé ; l'association `.cvw` pointe vers `bin\cvw.exe`.

Utilisation :

- [ ] Double-clic sur un `.cvw` dans l'Explorateur : Centre-Ville l'ouvre (titre « Centre-Ville - <fichier> »).
- [ ] Icône des fichiers `.cvw` dans l'Explorateur : celle de l'original (Icon_1).
- [ ] Fichier › Ouvrir : la boîte démarre dans `%LOCALAPPDATA%\Patquoi.fr\Centre-Ville`.
- [ ] F1 sur la fenêtre principale, puis dans Paramètres, Rechercher, une fiche de distances : le navigateur s'ouvre **à la bonne rubrique** (pas en haut de la page).
- [ ] Aide › Sommaire, Aide › Tutoriel.
- [ ] Boutons des boîtes de message en français (« Oui », « Non », « Annuler »).
- [ ] Écran à 125 %, 150 % et 200 % : images nettes, fiches lisibles.
- [ ] `cvw.exe !` (raccourci modifié) : `cvw.log` écrit dans `%LOCALAPPDATA%\Patquoi.fr\Centre-Ville` à la fermeture.
- [ ] Désinstallation : `C:\Program Files\Patquoi.fr\Centre-Ville` supprimé, cartes de l'utilisateur gardées.

## macOS 12 ou plus récent (Apple Silicon, et Intel si possible)

Paquet : `Centre-Ville-<version>-macOS.dmg`.

- [ ] Le `.dmg` s'ouvre : Centre-Ville, raccourci Applications, dossier `Cartes`, `licence.txt`.
- [ ] Centre-Ville glissé dans Applications ; premier lancement refusé (application non notarisée), puis Réglages Système › Confidentialité et sécurité › « Ouvrir quand même » : l'application démarre. Variante : `xattr -dr com.apple.quarantine /Applications/Centre-Ville.app`.
- [ ] Le message n'annonce pas une application « endommagée » (signature ad hoc valide).
- [ ] « À propos » : « 6,0 » « ,0 » ; menu Centre-Ville › À propos de Centre-Ville.
- [ ] Double-clic sur un `.cvw` (dossier `Cartes`) : Centre-Ville l'ouvre ; icône des `.cvw` dans le Finder.
- [ ] Premier lancement : les 6 cartes sont copiées dans `~/Library/Application Support/Patquoi.fr/Centre-Ville` ; une carte déjà présente n'est pas écrasée ; une carte supprimée n'est pas recopiée au lancement suivant.
- [ ] Fichier › Ouvrir démarre dans ce dossier et montre les cartes.
- [ ] Carte ouverte en lecture seule, depuis le volume monté (`hdiutil attach -readonly`, ou double-clic sur le `.dmg`) : ouvrir `Cartes/Géant.cvw` du volume, Marche : la simulation démarre sans message et sans rien enregistrer ; la laisser tourner, Arrêt : bilan, puis « Chargement effectué. », réseau d'origine ; recommencer Marche et Arrêt.
- [ ] Même carte, modifiée (Paramètres, par exemple), Marche : Centre-Ville dit que le fichier est en lecture seule et propose un autre nom. « Non » : pas de démarrage, modification gardée, Arrêt grisé ; nouveau clic sur Marche : même question, sans blocage. « Oui », puis un nom dans le dossier proposé : la simulation démarre, et Arrêt recharge ce fichier.
- [ ] `codesign --verify --deep --strict` sur le paquet, après ces lancements : signature toujours valide.

Le parcours « non modifié » est lancé par le workflow « Installeurs », en « offscreen », avant chaque release (job « Carte du .dmg en lecture seule »). Pour le refaire dans l'affichage natif, sur la carte du volume monté, depuis une compilation avec les tests :

```bash
CV_CARTE="$PWD/maps/Démo.cvw" CV_CARTE_LECTURE_SEULE="/Volumes/Centre-Ville <version>/Cartes/Géant.cvw" QT_QPA_PLATFORM=cocoa build/release/tests/test_interface carteDUnVolumeEnLectureSeule
```
- [ ] F1 (fenêtre principale, Paramètres, Rechercher) : rubrique exacte dans le navigateur par défaut, avec Safari et avec Chrome.
- [ ] Raccourcis : ⌘ à la place de Ctrl ; Control+Q (Carrefours) et Control+H (Rechercher).
- [ ] Thème clair et thème sombre ; écran Retina et écran non Retina.
- [ ] Mac Intel : l'application universelle démarre (`lipo -archs` : `x86_64 arm64`).

## Linux x86_64 (Ubuntu 22.04 ou 24.04, Fedora, Debian 12)

Paquet : `Centre-Ville-<version>-x86_64.AppImage`.

- [ ] `chmod +x`, puis lancement : l'application démarre (X11 et Wayland/XWayland).
- [ ] Sans FUSE (`libfuse2` absent) : `--appimage-extract-and-run` fonctionne.
- [ ] Premier lancement : les 6 cartes sont copiées dans `~/.local/share/Patquoi.fr/Centre-Ville` ; une carte déjà présente n'est pas écrasée ; une carte supprimée n'est pas recopiée au lancement suivant.
- [ ] Fichier › Ouvrir démarre dans ce dossier et montre les cartes.
- [ ] F1 : rubrique exacte avec un navigateur non confiné (Firefox ou Chrome installés en `.deb`/`.rpm`) ; noter le comportement avec le Firefox snap d'Ubuntu (limite connue).
- [ ] Boutons des boîtes de message en français.
- [ ] Facultatif : intégration par AppImageLauncher ou Gear Lever (entrée de menu, icône, association des `.cvw`).

## Partout : fonctionnement (phase 9 de PLAN_PORTAGE.md)

- [ ] Chaque menu, action, raccourci, barre d'outils et fiche s'ouvre et se ferme.
- [ ] Les 6 cartes de `maps/` simulées un moment (Géant : le blocage d'origine des vehlibs peut figer la simulation selon le tirage, bogue connu).
- [ ] Épidémie ; mode sanglant et résurrection ; génération aléatoire ; « Copier vers… » ; vérification d'intégrité ; statistiques et recherche.
- [ ] Interopérabilité : un `.cvw` enregistré par la 6.0.0 s'ouvre dans la 5.4.3.2 d'origine (Windows ou Wine), et inversement.
- [ ] Comparaison visuelle avec les captures du guide (`docs/guide`) et avec l'original.
