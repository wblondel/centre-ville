# Centre-Ville 🚦🚗

[![License: GPL v2](https://img.shields.io/badge/License-GPLv2-blue.svg)](LICENSE)
[![GitHub Pages](https://img.shields.io/badge/Docs-GitHub%20Pages-success.svg)](https://wblondel.github.io/ProjetCentreVille/)
[![Platform: Windows | macOS | Linux](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-0078D6.svg)](#-installation--ex%C3%A9cution)

**Centre-Ville** est un logiciel de simulation de circulation urbaine complet, interactif et pédagogique créé par **Patrice Fouquet** ([patquoi.fr](http://patquoi.fr#cvw)). Initialement développé pour Windows (Windows 95/XP jusqu'à Windows 10/11), il est distribué sous licence **GNU General Public License v2.0 (GPLv2)**.

La version **6.0.0** est la version officielle finale **5.4.3.2**, portée de C++Builder (Windows seulement) vers **C++20, CMake et Qt 6**, et native sous Windows, macOS et Linux : même moteur de simulation, mêmes fenêtres, menus et raccourcis, même aide, même format de fichier `.cvw` (voir [`PLAN_PORTAGE.md`](PLAN_PORTAGE.md) et [`CHANGELOG.md`](CHANGELOG.md)). La version **6.0.1** corrige les bogues d'origine du moteur, dont le blocage de la simulation par les vehlibs et leurs passagers qui n'en descendaient jamais.

---

## 🌟 Fonctionnalités

* **Réseau routier modulaire :** Voies simples, doubles voies, périphériques, sens uniques, ronds-points et carrefours complexes.
* **Système de feux tricolores avancé :** Cycles configurables, feux asservis, détection et synchronisation.
* **Transports en commun :**
  * Lignes de bus et dépôts, arrêts configurables.
  * Lignes de tramways, aiguillages et voies dédiées.
  * Taxis avec bornes d'attente et files dédiées.
* **Piétons & Traversées :** Passages protégés, flux piétons et interactions avec les véhicules.
* **Mode Épidémie :** Modélisation de la propagation d'un virus parmi les piétons avec taux de contagion, période d'incubation et réinfections.
* **Statistiques & Diagnostic :** Suivi des temps de parcours, détection des points de congestion et analyse des flux.

---

## 📂 Structure du Répertoire

```text
.
├── .github/
│   └── workflows/
│       ├── tests-moteur.yml      # CI : tests du moteur et de l'application sur les 3 systèmes
│       ├── build-installer.yml   # Installeurs (Windows, macOS, Linux) et release sur tag v*
│       └── deploy-docs.yml       # CI Pages : déploiement automatique de la documentation
├── app/                          # Application Qt 6 (cvw) : les 33 fiches portées, couche VCL (app/vcl)
├── engine/                       # Moteur de simulation (C++20, sans Qt)
├── tests/                        # Tests du moteur (golden master) et de l'interface
├── tools/                        # Conversion des .dfm (dfm2ui.py), extraction des images et icônes, audit des bornes (audit_bornes.py)
├── docs/                         # Documentation en ligne (Guide & Tutoriels)
│   ├── index.html                # Portail d'accueil moderne
│   ├── guide/                    # Manuel complet d'utilisation (HTML), aide F1 de l'application
│   ├── tutorial/                 # Tutoriel pas-à-pas illustré
│   └── RECETTE.md                # Liste de recette d'une version (Windows, macOS, Linux)
├── inno-setup/                   # Script d'installation Inno Setup (inscvw95.iss)
├── maps/                         # Cartes de simulation d'exemple (*.cvw)
├── res/                          # Icônes et bitmaps graphiques haute résolution
├── src/                          # Code VCL d'origine (C++Builder), en lecture seule : voir plus bas
├── CHANGELOG.md                  # Journal des versions (notes des releases), bogues d'origine conservés
├── CMakeLists.txt                # Projet CMake (numéro de version)
├── CMakePresets.json
├── PLAN_PORTAGE.md               # Plan et état du portage
├── LICENSE                       # Licence GPL v2
└── README.md
```

---

## 🚀 Installation & Exécution

Les paquets sont joints aux [Releases](../../releases) (sans signature : le système affiche un avertissement au premier lancement). Les versions marquées « Pre-release » (par exemple `6.0.0-rc1`) sont des versions candidates, à essayer avant la version finale.

### Windows 10 / 11 (64 bits)
Téléchargez `Centre-Ville-<version>-Windows-x64-Setup.exe` et suivez l'assistant Inno Setup : il installe Centre-Ville, son aide et ses cartes d'exemple (dans `%LOCALAPPDATA%\Patquoi.fr\Centre-Ville`) et associe les fichiers `.cvw`. Il met à jour une version 5.4.3.2 d'origine déjà installée (même identifiant d'application). Si SmartScreen bloque l'installeur : « Informations complémentaires », puis « Exécuter quand même ». La version 32 bits n'existe plus (Qt 6) ; Windows ARM64 exécute la version x64.

### macOS 12 ou plus récent (Apple Silicon et Intel)
Ouvrez `Centre-Ville-<version>-macOS.dmg` et glissez **Centre-Ville** dans **Applications** ; le dossier `Cartes` contient les cartes d'exemple. L'application n'étant pas notarisée, macOS refuse le premier lancement : ouvrez **Réglages Système › Confidentialité et sécurité**, puis « Ouvrir quand même », ou retirez la quarantaine :
```bash
xattr -dr com.apple.quarantine /Applications/Centre-Ville.app
```

### Linux (x86_64)
Téléchargez `Centre-Ville-<version>-x86_64.AppImage`, rendez-la exécutable et lancez-la :
```bash
chmod +x Centre-Ville-*-x86_64.AppImage
./Centre-Ville-*-x86_64.AppImage
```
Au premier lancement, l'AppImage copie les cartes d'exemple dans `~/.local/share/Patquoi.fr/Centre-Ville` (sans écraser un fichier existant), où « Ouvrir » et « Enregistrer » démarrent, comme l'installeur Windows dans `%LOCALAPPDATA%`. Il faut une distribution x86_64 avec glibc 2.35 ou plus récente (Ubuntu 22.04, Debian 12…). Sans FUSE, ajoutez l'option `--appimage-extract-and-run`. L'association des fichiers `.cvw` demande un outil d'intégration des AppImage (AppImageLauncher, Gear Lever) ou une installation par CMake (ci-dessous). Avec un navigateur confiné (snap ou Flatpak, comme le Firefox d'Ubuntu), l'aide de l'AppImage ne s'ouvre pas : utilisez un navigateur installé autrement, ou l'aide en ligne.

---

## 🛠️ Compilation & Intégration Continue (CI/CD)

### 1. Compilation depuis les sources
Prérequis : compilateur C++20 (Clang, GCC 12 ou plus, MSVC 2022), CMake ≥ 3.25, Ninja (préréglages `dev` et `release`) et **Qt 6.5 ou plus récent** (Widgets ; Qt 6.8 LTS en CI ; macOS : `brew install qt`).
```bash
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
```
L'application est `build/dev/app/cvw` (`cvw.exe` sous Windows, `Centre-Ville.app` sous macOS) ; son aide (`html/`, `tutocv/`) est copiée à côté. Le moteur seul se compile sans Qt (`-DCV_APP=OFF`). Détails : [`app/README.md`](app/README.md) et [`engine/README.md`](engine/README.md).

### 2. Installation et paquets
```bash
cmake --preset release -DCV_TESTS=OFF && cmake --build --preset release
cmake --install build/release --prefix "$PWD/build/installeur"   # windeployqt / macdeployqt (préfixe absolu)
```
* **Windows** : `iscc /DStaging=<chemin absolu de build\installeur> inno-setup\inscvw95.iss` (Inno Setup ≥ 6.3).
* **macOS** : `cd build/release && cpack` produit `Centre-Ville-<version>-macOS.dmg`.
* **Linux** : AppImage avec [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) et son greffon Qt, comme dans `build-installer.yml` (le crochet `app/linux/apprun-hooks/centre-ville-cartes.sh` copie les cartes au premier lancement) ; ou installation dans un préfixe dédié (`--prefix /opt/centre-ville`), avec entrée de bureau, type MIME des fichiers `.cvw` et cartes dans `share/centre-ville/cartes`.

Le numéro de version est celui de `project(VERSION …)` dans `CMakeLists.txt` (affiché par « À propos » : « 6,0 » et « ,2 »).

### 3. Intégration continue
* `.github/workflows/tests-moteur.yml` : moteur et application compilés et testés sous Windows, macOS et Linux à chaque modification, et toute la suite sous AddressSanitizer et UBSan (macOS et Linux).
* `.github/workflows/build-installer.yml` : sur un tag `v*` ou à la main, compile depuis les sources et produit l'installeur Windows, le `.dmg`, l'AppImage et les archives des sources ; sur un tag seulement, les joint à une release GitHub (voir ci-dessous).
* `.github/workflows/recette-windows.yml` : avant chaque release (ou à la main), installe le paquet Windows sur une machine Windows de GitHub, neuve ou avec la 5.4.3.2 d'origine (mise à jour), ouvre un `.cvw`, essaie F1 et la désinstallation, et joint des captures d'écran (recette sans machine Windows).
* `.github/dependabot.yml` : mises à jour mensuelles et groupées des actions et des outils de lint.
* `.github/workflows/deploy-docs.yml` : publie le dossier `docs/` sur **GitHub Pages** à chaque mise à jour de `main` (guide : `<url-pages>/guide/index.html`, tutoriel : `<url-pages>/tutorial/index.html`).

### 4. Lint
Les versions sont dans `tools/lint/requirements.txt`, et la CI lance les mêmes commandes :

```bash
python3 -m venv .venv-lint && .venv-lint/bin/pip install -r tools/lint/requirements.txt
.venv-lint/bin/ruff check tools
PATH=".venv-lint/bin:$PATH" actionlint
.venv-lint/bin/zizmor .github/workflows
cmake --preset dev && cmake --build --preset dev
.venv-lint/bin/python tools/lint_cpp.py --dossier build/dev --clang-tidy .venv-lint/bin/clang-tidy
```

`clang-tidy` ne fait échouer que les constats absents de `tools/lint/clang-tidy-reference.txt`, qui liste ceux du code porté et fait foi sous Linux. Fausse alerte : `// NOLINT(regle)` sur la ligne, avec le motif. Le plan d'amélioration de la CI est dans [`PLAN_CI.md`](PLAN_CI.md).

### 5. Publier une version
1. Mettre le numéro dans `project(VERSION …)` de `CMakeLists.txt` et une section `## [<version>]` dans `CHANGELOG.md` (elle devient les notes de la release).
2. Pousser un tag : `v<version>` publie une release, `v<version>-rc1` (ou tout autre suffixe) une **pré-release**, facultative. Un tag qui ne correspond pas à `project(VERSION)`, ou une version sans section dans `CHANGELOG.md`, arrête le workflow avant toute compilation.
3. Le workflow vérifie les paquets, démarre une simulation depuis une carte du `.dmg` monté en lecture seule et passe la recette Windows sur l'installeur. Un échec empêche la release. Lancé à la main, il fait tout cela sans rien publier.
4. Les releases sont immuables : une fois publiée, ni ses paquets ni son tag ne changent. Une release ratée se corrige par une nouvelle version, et son nom de tag ne resservira pas.
5. Chaque release joint `SHA256SUMS` et une attestation de provenance : `gh attestation verify <paquet> --repo wblondel/centre-ville`.
6. Ce qu'une machine ne couvre pas reste à faire à la main si besoin : [`docs/RECETTE.md`](docs/RECETTE.md).

### 6. Code d'origine (`src/`)
`src/` contient le code VCL d'origine de la version 5.4.3.2 (C++Builder ; `src/cvw.cbproj` s'ouvre dans RAD Studio 10.3 / 10.4). Il est en **lecture seule** : on ne le modifie pas, mais il sert encore de source au portage :
* `tools/dfm2ui.py` produit les fiches Qt (`app/ui/*.ui`) depuis ses `.dfm` ;
* `tools/extrait_images.py` en extrait les images (`app/ressources`) ;
* le test de l'aide (`tests/app/test_aide.cpp`) lit la table des rubriques de `src/base.cpp` ;
* deux contrôles de la CI (`tests-moteur.yml`) vérifient que les fiches `.ui` et les images extraites sont à jour.

Le port garde les noms de fichiers et de fonctions de `src/` : `git diff --no-index --word-diff src/f_simul.cpp app/f_simul.cpp` montre le portage ligne à ligne.

---

## 📊 État du portage

| Phase ([`PLAN_PORTAGE.md`](PLAN_PORTAGE.md)) | État |
|---|---|
| 0. Préparation et oracle | En partie : binaire officiel désassemblé (générateur aléatoire, ordre des tirages) ; pas d'original instrumenté. |
| 1-2. Moteur autonome et tests (golden master) | Fait (`engine/`, `tests/`). |
| 3-5. Ressources, grille de simulation, fiche principale | Fait. |
| 6. Les 32 fiches secondaires | Fait (les 33 fiches sont portées). |
| 7. Services système (aide F1, journal) | Fait. |
| 8. Build, CI et distribution | Fait : installeurs Windows, macOS et Linux compilés par la CI. |
| 9. Recette et bascule | Fait : version 6.0.0 après trois pré-releases. Recette automatisée sous Windows (installation, mise à jour d'une 5.4.3.2, interopérabilité avec l'original, fiches en affichage natif : workflow « Recette Windows »), sous macOS et sous Linux ; restent à faire à la main, sur de vrais postes, les points de [`docs/RECETTE.md`](docs/RECETTE.md) que ces machines ne couvrent pas (SmartScreen, Gatekeeper, Windows 10/11 grand public et ARM64). |

Les écarts avec l'original et les bogues d'origine corrigés (moteur en 6.0.1, interface en 6.0.2) sont listés dans [`CHANGELOG.md`](CHANGELOG.md), [`engine/README.md`](engine/README.md) et [`app/README.md`](app/README.md).

---

## 🗺️ Feuille de Route & Modernisation

Le cœur de simulation ([`engine/simul.cpp`](engine/simul.cpp), port de [`src/simul.cpp`](src/simul.cpp)) est découplé de l'interface ; l'interface est portée sur Qt 6 ([`PLAN_PORTAGE.md`](PLAN_PORTAGE.md)).

* [x] Découplage complet du moteur de simulation de la couche VCL.
* [x] Portage vers un framework multiplateforme moderne (**Qt 6** avec CMake).
* [x] Compilation native sous Linux et macOS sans émulation Wine.
* [x] Installeurs Windows, macOS et Linux compilés depuis les sources.
* [x] Recette de la version 6.0.0 sur les trois systèmes, puis bascule (phase 9).
* [x] Après la bascule : corriger les bogues d'origine du moteur, dont le blocage des vehlibs sur Géant (version 6.0.1).
* [x] Corriger les bogues d'origine de l'interface (version 6.0.2, [`CHANGELOG.md`](CHANGELOG.md)).
* [x] Corriger les plantages trouvés ensuite et les derniers comportements gênants de l'interface (version 6.0.3).
* [x] Tester le parcours d'édition d'une carte par l'interface, et corriger les pertes de modifications trouvées (version 6.0.4).
* [x] Tester les fiches Feux par l'interface, et corriger la suppression d'un feu référencé (version 6.0.5).
* [x] Mettre en place un linter dans la CI (version 6.0.6, [`PLAN_CI.md`](PLAN_CI.md)).
* [ ] Signature et notarisation des paquets (Windows, macOS).
* [ ] Traduction / localisation de l'interface en anglais.
* [ ] Portage WebAssembly pour exécuter la simulation directement dans le navigateur.

---

## 📄 Licence & Crédits

* **Auteur original :** Patrice Fouquet ([patquoi.fr](http://patquoi.fr))
* **Licence :** Distribué sous les termes de la [GNU General Public License v2.0](LICENSE).
