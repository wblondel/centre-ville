# Centre-Ville 🚦🚗

[![License: GPL v2](https://img.shields.io/badge/License-GPLv2-blue.svg)](LICENSE)
[![GitHub Pages](https://img.shields.io/badge/Docs-GitHub%20Pages-success.svg)](https://wblondel.github.io/ProjetCentreVille/)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows%20%7C%20Wine-0078D6.svg)](#installation--running)

**Centre-Ville** est un logiciel de simulation de circulation urbaine complet, interactif et pédagogique créé par **Patrice Fouquet** ([patquoi.fr](http://patquoi.fr#cvw)). Initialement développé pour Windows (Windows 95/XP jusqu'à Windows 10/11), il est distribué sous licence **GNU General Public License v2.0 (GPLv2)**.

Cette version correspond à la version officielle finale **v5.4.3.2**.

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
│       ├── build-installer.yml   # CI Windows : compilation Inno Setup & Release
│       └── deploy-docs.yml       # CI Pages : déploiement automatique de la documentation
├── docs/                         # Documentation en ligne (Guide & Tutoriels)
│   ├── index.html                # Portail d'accueil moderne
│   ├── guide/                    # Manuel complet d'utilisation (HTML)
│   └── tutorial/                 # Tutoriel pas-à-pas illustré
├── inno-setup/                   # Script d'installation Inno Setup (inscvw95.iss)
├── maps/                         # Cartes de simulation d'exemple (*.cvw)
├── res/                          # Icônes et bitmaps graphiques haute résolution
├── src/                          # Code source complet C++ (C++Builder VCL)
│   ├── simul.cpp / simul.h       # Moteur central de simulation (logique et IA)
│   ├── f_simul.cpp               # Interface graphique principale
│   └── cvw.cbproj                # Projet Embarcadero C++Builder
├── legacy/                       # Archives historiques (portages Delphi v3.1, Kylix Linux v4.3.1)
├── LICENSE                       # Licence GPL v2
└── README.md
```

---

## 🚀 Installation & Exécution

### Sous Windows (XP, Vista, 7, 8, 10, 11)
1. **Via l'installeur :** Téléchargez le fichier `Centre-Ville_Setup_v5.4.3.2.exe` depuis la section [Releases](../../releases) et suivez l'assistant Inno Setup.
2. **Version portable :** Téléchargez l'archive `Centre-Ville_Portable_v5.4.3.2.zip`, décompressez-la et lancez `bin/cvw.64.exe` (ou `bin/cvw.32.exe` sur les systèmes 32 bits).

### Sous macOS / Linux (via Wine)
Centre-Ville fonctionne parfaitement sous Wine, Bottles ou Whisky :
```bash
# Exemple avec Wine sur macOS / Linux
wine bin/cvw.exe
```

---

## 🛠️ Compilation & Intégration Continue (CI/CD)

### 1. Documentation en ligne (GitHub Pages)
Le workflow `.github/workflows/deploy-docs.yml` publie automatiquement le contenu du dossier `docs/` sur **GitHub Pages** à chaque mise à jour de la branche principale `main`.
* **Guide complet :** `<url-pages>/guide/index.html`
* **Tutoriel :** `<url-pages>/tutorial/index.html`

### 2. Installeur Inno Setup
Le workflow `.github/workflows/build-installer.yml` s'exécute sur `windows-latest` lors de la publication d'un tag (`v*`) ou via déclenchement manuel :
1. Installe Inno Setup.
2. Génère l'archive source `cvw-src-win64.zip`.
3. Compile l'installeur `Centre-Ville_Setup_*.exe`.
4. Crée la release GitHub avec l'installeur et le paquet portable.

Pour compiler l'installeur localement sous Windows :
```cmd
cd inno-setup
iscc.exe inscvw95.iss
```

### 3. Compilation du code source C++
Le code source natif utilise **Embarcadero C++Builder** (RAD Studio 10.3 / 10.4) avec le framework VCL :
* Ouvrez `src/cvw.cbproj` dans C++Builder.
* Sélectionnez la plateforme cible (`Win64` ou `Win32`) et la configuration `Release`.
* Lancez la compilation (`Shift + F9`).

---

## 🗺️ Feuille de Route & Modernisation

Le cœur de simulation ([`src/simul.cpp`](src/simul.cpp) / [`src/simul.h`](src/simul.h)) est écrit en C++ standard et découplable de l'interface propriétaire VCL.

Les objectifs futurs de modernisation comprennent :
* [ ] Découplage complet du moteur de simulation de la couche VCL.
* [ ] Portage vers un framework multiplateforme moderne (**SDL2**, **Raylib** ou **wxWidgets** avec CMake).
* [ ] Compilation native sous Linux et macOS sans émulation Wine.
* [ ] Traduction / localisation de l'interface en anglais.
* [ ] Portage WebAssembly pour exécuter la simulation directement dans le navigateur.

---

## 📄 Licence & Crédits

* **Auteur original :** Patrice Fouquet ([patquoi.fr](http://patquoi.fr))
* **Licence :** Distribué sous les termes de la [GNU General Public License v2.0](LICENSE).
