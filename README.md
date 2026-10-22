# Centre-Ville 🚦🚗

[![License: GPL v2](https://img.shields.io/badge/License-GPLv2-blue.svg)](LICENSE)
[![GitHub Pages](https://img.shields.io/badge/Docs-GitHub%20Pages-success.svg)](https://wblondel.github.io/ProjetCentreVille/)
[![Platform: Windows | macOS | Linux](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-0078D6.svg)](#-installation--ex%C3%A9cution)

**Centre-Ville** est un logiciel de simulation de circulation urbaine complet, interactif et pédagogique créé par **Patrice Fouquet** ([patquoi.fr](http://patquoi.fr#cvw)). Initialement développé pour Windows (Windows 95/XP jusqu'à Windows 10/11), il est distribué sous licence **GNU General Public License v2.0 (GPLv2)**.

Cette version correspond à la version officielle finale **v5.4.3.2**, portée vers **C++20, CMake et Qt 6** pour Windows, macOS et Linux (voir [`PLAN_PORTAGE.md`](PLAN_PORTAGE.md)) : même moteur de simulation, mêmes fenêtres, même format de fichier `.cvw`.

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
├── tools/                        # Conversion des .dfm (dfm2ui.py), extraction des images et icônes
├── docs/                         # Documentation en ligne (Guide & Tutoriels)
│   ├── index.html                # Portail d'accueil moderne
│   ├── guide/                    # Manuel complet d'utilisation (HTML), aide F1 de l'application
│   └── tutorial/                 # Tutoriel pas-à-pas illustré
├── inno-setup/                   # Script d'installation Inno Setup (inscvw95.iss)
├── maps/                         # Cartes de simulation d'exemple (*.cvw)
├── res/                          # Icônes et bitmaps graphiques haute résolution
├── src/                          # Code source d'origine C++Builder VCL (référence du portage, lecture seule)
├── CMakeLists.txt                # Projet CMake (numéro de version)
├── CMakePresets.json
├── PLAN_PORTAGE.md               # Plan et état du portage
├── LICENSE                       # Licence GPL v2
└── README.md
```

---

## 🚀 Installation & Exécution

Les paquets sont joints aux [Releases](../../releases) (sans signature : le système affiche un avertissement au premier lancement).

### Windows 10 / 11 (64 bits)
Téléchargez `Centre-Ville-<version>-Windows-x64-Setup.exe` et suivez l'assistant Inno Setup : il installe Centre-Ville, son aide et ses cartes d'exemple (dans `%LOCALAPPDATA%\Patquoi.fr\Centre-Ville`) et associe les fichiers `.cvw`. Il remplace une version 5.4.3.2 d'origine déjà installée. Si SmartScreen bloque l'installeur : « Informations complémentaires », puis « Exécuter quand même ». La version 32 bits n'existe plus (Qt 6) ; Windows ARM64 exécute la version x64.

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
Les cartes d'exemple sont dans le dossier `maps/` de l'archive des sources. Sans FUSE, ajoutez l'option `--appimage-extract-and-run`. L'association des fichiers `.cvw` demande un outil d'intégration des AppImage (AppImageLauncher, Gear Lever) ou une installation par CMake (ci-dessous).

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
cmake --install build/release --prefix build/installeur   # windeployqt / macdeployqt
```
* **Windows** : `iscc /DStaging=<chemin absolu de build\installeur> inno-setup\inscvw95.iss` (Inno Setup ≥ 6.3).
* **macOS** : `cd build/release && cpack` produit `Centre-Ville-<version>-macOS.dmg`.
* **Linux** : AppImage avec [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) et son greffon Qt, comme dans `build-installer.yml` ; ou installation dans un préfixe dédié (`--prefix /opt/centre-ville`), avec entrée de bureau et type MIME des fichiers `.cvw`.

Le numéro de version est celui de `project(VERSION …)` dans `CMakeLists.txt`.

### 3. Intégration continue
* `.github/workflows/tests-moteur.yml` : moteur et application compilés et testés sous Windows, macOS et Linux à chaque modification.
* `.github/workflows/build-installer.yml` : sur un tag `v*` ou à la main, compile depuis les sources et produit l'installeur Windows, le `.dmg`, l'AppImage et les archives des sources ; sur un tag seulement, les joint à une release GitHub.
* `.github/workflows/deploy-docs.yml` : publie le dossier `docs/` sur **GitHub Pages** à chaque mise à jour de `main` (guide : `<url-pages>/guide/index.html`, tutoriel : `<url-pages>/tutorial/index.html`).

### 4. Code d'origine (C++Builder)
Le code VCL d'origine reste dans `src/` (référence du portage) : `src/cvw.cbproj` s'ouvre dans Embarcadero C++Builder (RAD Studio 10.3 / 10.4).

---

## 🗺️ Feuille de Route & Modernisation

Le cœur de simulation ([`engine/simul.cpp`](engine/simul.cpp), port de [`src/simul.cpp`](src/simul.cpp)) est découplé de l'interface ; l'interface est portée sur Qt 6 ([`PLAN_PORTAGE.md`](PLAN_PORTAGE.md)).

* [x] Découplage complet du moteur de simulation de la couche VCL.
* [x] Portage vers un framework multiplateforme moderne (**Qt 6** avec CMake).
* [x] Compilation native sous Linux et macOS sans émulation Wine.
* [ ] Recette complète et bascule (phase 9 du plan de portage).
* [ ] Traduction / localisation de l'interface en anglais.
* [ ] Portage WebAssembly pour exécuter la simulation directement dans le navigateur.

---

## 📄 Licence & Crédits

* **Auteur original :** Patrice Fouquet ([patquoi.fr](http://patquoi.fr))
* **Licence :** Distribué sous les termes de la [GNU General Public License v2.0](LICENSE).
