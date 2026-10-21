# Plan de portage — Centre-Ville 5.4.3.2 vers une stack moderne

## 0. Objectif et contraintes

- **Logique intacte** : mêmes règles de simulation, mêmes paramètres, même format `.cvw` (lecture des anciennes versions, écriture en `0x530` octet pour octet).
- **Interface intacte** : mêmes fenêtres, menus, barres d'outils, raccourcis, sprites, textes, dispositions, comportements (modal / non modal).
- **Compilable sans outil propriétaire**, nativement sous Windows, macOS et Linux.
- **Règle d'or : on porte, on ne réécrit pas.** Aucune correction de bug ni amélioration pendant le portage : on les note dans `src/A_FAIRE.txt` pour après la bascule.

---

## 1. État des lieux (mesuré dans le code)

| Bloc | Volume | Points de couplage |
|---|---|---|
| Moteur `simul.cpp` / `simul.h` | ~8 000 lignes | C++ quasi standard. Dépendances Borland/VCL : `AnsiString`, `Format(…, ARRAYOFCONST(…))` (106 appels), `IntToStr` (34), `FileRead`/`FileWrite` sur handle `int` (128 + 128), `TDateTime`/`Time()` (chronométrage), `rand()`/`srand()` (28 appels). **Appelle directement l'UI** : inclut 9 en-têtes de fiches, `frmSimulation->RedessineCase` (34), `frmSimulation->StatusBar` (24), `frmRecherche->…` (~20), `frmInfos*->RafraichitInfos`, `frmStatistiques`, etc. |
| Interface : 33 fiches (`.dfm` texte + `.cpp`/`.h`) | ~12 000 lignes | `f_simul.cpp` (3 565 lignes) = fiche principale. ~1 300 composants : 472 `TLabel`, 238 `TCSpinEdit`, 202 `TMenuItem`, 100 `TAction`, 68 `TButton`, 57 `TToolButton`, 34 `TBitBtn`, 34 `TCheckBox`, 29 `TGroupBox`, 28 `TRadioButton`, 26 `TSpeedButton`, 20 `TShape`, 16 `TImage`, 13 `TRadioGroup`, 13 `TPopupMenu`, 8 `TStringGrid`, 7 `TToolBar`, 6 `TTimer`, 5 `TTrackBar`, 2 `TPaintBox`, 1 `TDrawGrid`. Extensions Borland : 745 `__fastcall`, 10 `__finally`, 5 `__property`. |
| Rendu de la carte | — | `TDrawGrid` 99×63 cases de 16 px. Chaque case = empilement de sprites (`TVirtualImageList` ← `TImageCollection` dont les PNG sont **embarqués en hexadécimal dans `f_simul.dfm`** : 178 boutons, 34 glyphes, 976 entrées « Simulation »), plus quelques primitives `Canvas` (lignes, rectangles, texte). Rafraîchissement partiel via `InvalidateRect` par case. |
| Boucle de simulation | — | `TimerSimul` → `cv->EffectueTourSimulation()` à chaque tick, garde de réentrance `cv->TourEnCours`, demandes pause/arrêt via `TypeDemande`. |
| Spécifique Windows | `base.cpp`, `f_propos.cpp`, `cvw.cpp` | `MessageBoxW`, `TRegistry` + `CreateProcess` (ouvrir l'aide HTML), `ShellExecute`, `%LOCALAPPDATA%\Patquoi.fr\Centre-Ville\`, `WinMain`. |
| Format `.cvw` | 6 cartes dans `maps/` | Binaire brut little-endian, versionné (`0x200` → `0x530`). `int` = 4 octets, énumérés forcés sur 1 octet depuis la v5.4 (`enum … : char`), `nature` = 4 octets, `bool` = 1 octet. |
| Aide | `docs/guide`, `docs/tutorial` | Déjà en HTML. Table `HelpContext` → ancre dans `base.cpp`. |

---

## 2. Stack cible recommandée : **C++20 + CMake + Qt 6 Widgets**

**Pourquoi ce choix :**

1. **Le moteur reste en C++** : on le recompile au lieu de le traduire. C'est la seule option qui préserve la logique *par construction* plutôt que par relecture.
2. **Qt Widgets est l'équivalent le plus proche de la VCL** (correspondance quasi 1:1, voir §6) et accepte le positionnement absolu → les fiches peuvent être reproduites au pixel près.
3. Les `.dfm` sont en texte : conversion **automatisable** vers des `.ui` Qt.
4. Natif sur les 3 OS, HiDPI géré ; `QSettings`, `QStandardPaths`, `QDesktopServices` remplacent registre et `ShellExecute`.
5. Licence : `qtbase` est disponible sous GPLv2 → compatible avec la licence du projet.
6. Ouvre la porte au portage WebAssembly prévu dans le README (Qt for WebAssembly), sans nouveau portage.

**Outils** : Qt 6.8 LTS, CMake ≥ 3.25 + Ninja, QtTest (ou Catch2) pour les tests, Clang/GCC/MSVC.

**Options écartées :**

| Option | Raison |
|---|---|
| wxWidgets | Viable (rendu natif), mais conversion des fiches et contrôles (spin, grille dessinée) moins outillés. Second choix. |
| Lazarus / LCL | Clone de la VCL qui importe les `.dfm`… mais en Pascal : il faudrait traduire les 20 000 lignes de C++. |
| Web (TypeScript/Canvas), Tauri, Rust/egui, Godot | Réécriture complète du moteur → risque élevé de dérive de logique, interface à recréer. |

---

## 3. Architecture cible

```text
ProjetCentreVille/
├── CMakeLists.txt
├── CMakePresets.json
├── engine/                 # bibliothèque statique cvengine, SANS Qt
│   ├── simul.h / simul.cpp # moteur quasi inchangé
│   ├── vue.h               # interface IVueSimulation (callbacks vers l'UI)
│   └── compat/             # shims minimaux pour garder simul.cpp intact
│       ├── chaine.h        #   AnsiString (sous-ensemble utilisé), Format, IntToStr
│       ├── fichier.h       #   FileRead/FileWrite/FileOpen/FileCreate/FileClose
│       ├── alea.h          #   rand/srand façon Borland (voir §4.1)
│       └── temps.h         #   TDateTime/Time() via std::chrono
├── app/                    # exécutable Qt
│   ├── main.cpp            # équivalent de cvw.cpp (création des fiches dans le même ordre)
│   ├── ui/*.ui             # générés depuis les .dfm, puis maintenus à la main
│   ├── f_*.cpp / f_*.h     # code des fiches porté (mêmes noms de fichiers et de méthodes)
│   ├── grille/             # widget de la carte (remplace TDrawGrid)
│   └── ressources/         # sprites extraits des .dfm + cvw.qrc
├── tests/                  # tests du moteur (round-trip .cvw, golden master)
├── tools/                  # dfm2ui.py, extrait_images.py
└── src/                    # code VCL d'origine, en lecture seule jusqu'à la bascule
```

Conserver les noms français, la structure des fonctions et les commentaires de version (`// v5.3 …`) : un diff de portage doit rester lisible ligne à ligne face à l'original.

---

## 4. Points techniques critiques

### 4.1 Générateur aléatoire
Les `rand()` de la libc diffèrent selon les plateformes (algorithme et `RAND_MAX`). Les expressions du type `rand()%100` ou `rand()%NbX` dépendent de `RAND_MAX`.
→ Réimplémenter l'LCG de la RTL Borland (`graine = graine * 22695477 + 1`, résultat = bits 30..16, `RAND_MAX = 32767`) dans `compat/alea.h`, **à vérifier contre le binaire d'origine** (voir Phase 0). Graine injectable pour les tests. `srand(time())` (simul.cpp:202) reste le comportement par défaut.

### 4.2 Format de fichier `.cvw`
Les lectures/écritures se font par `sizeof(champ)` → dépend du compilateur.
→ Garder les appels `FileRead(hfile, &x, sizeof(x))` mais :
- ajouter des `static_assert(sizeof(…) == …)` sur chaque type sérialisé (`int` = 4, énumérés = 1, `nature` = 4, `bool` = 1) ;
- **attention au `char` non signé sur Linux ARM64** : forcer `signed char` comme type sous-jacent des énumérés, ou compiler avec `-fsigned-char` ;
- vérifier qu'aucun `long` n'est sérialisé (`long` fait 8 octets sous Linux/macOS, 4 sous Windows) ;
- conserver `VERSION 0x530` : le format ne change pas.

### 4.3 Découplage moteur ↔ interface
Remplacer chaque `frmXxx->…` du moteur par un appel à une interface, **un pour un**, sans toucher à l'ordre des appels :

```cpp
struct IVueSimulation {
  virtual void RedessineCase(int x, int y, bool EffaceFond) = 0;
  virtual void AfficheStatut(const AnsiString &asTexte) = 0;      // StatusBar->Panels->Items[0]
  virtual bool SimulationSanglante() const = 0;
  virtual bool ResurrectionDemandee() const = 0;
  virtual bool EpidemieActivee() const = 0;
  virtual void RafraichitFichesInfos() = 0;                        // frmInfos*, frmStats*, si visibles
  virtual void RafraichitRecherche() = 0;                          // bloc frmRecherche (simul.cpp:6056)
  // … compléter d'après l'inventaire exhaustif des 34 + 24 + ~30 appels
};
```

Une implémentation `VueNulle` sert aux tests. `voie::RedessineCase` délègue à la vue au lieu de `frmSimulation`.

### 4.4 Comportements implicites de la VCL à reproduire
Ce sont eux qui cassent « l'interface intacte » s'ils sont oubliés :
- `TCSpinEdit->Value = …` **déclenche `OnChange`** (utilisé dans `f_prmsim.cpp` et ailleurs) → `OnChange` suit le texte du champ (`QLineEdit::textChanged`, signaux non bloqués) ; comme dans `CSPIN.pas`, `MinValue`/`MaxValue` ne bornent qu'à `Value=`, aux flèches et en quittant le champ, jamais pendant la frappe.
- `TCheckBox->Checked = …` et `TRadioButton->Checked = …` **déclenchent `OnClick`** → connecter `toggled`, pas `clicked`.
- `Application->CreateForm` crée **toutes les fiches au démarrage** (ordre dans `cvw.cpp`) et le code teste `frmX && frmX->Visible` → instancier les fiches au lancement, dans le même ordre.
- `TActionList` : `RafraichitActionsEdition` filtre par `Category` → stocker la catégorie dans `QAction::setProperty("categorie", …)`.
- `TSpeedButton` avec `GroupIndex` → `QToolButton` checkable dans un `QButtonGroup` exclusif.
- `MessageDlg` / `AfficheMessage` : mêmes boutons, même bouton par défaut, même bouton Échap ; boutons en français (charger `qtbase_fr.qm`).
- `__finally` → garde RAII (*scope guard*).

---

## 5. Phases

Taille relative : **S** petit, **M** moyen, **L** gros. Les phases 4 à 6 représentent environ les deux tiers de l'effort.

### Phase 0 — Préparation et oracle (S)
1. Branche `port/qt6`.
2. Récupérer le binaire officiel 5.4.3.2 (installeur patquoi.fr) et le faire tourner sous Wine/CrossOver ou dans une VM Windows → **oracle visuel** : capture de chaque fiche, menu, popup, état de barre d'outils.
3. *(Fortement recommandé)* Installer **C++Builder Community Edition** (gratuit) dans une VM Windows, uniquement pour produire une **version instrumentée de l'original** : graine `srand` fixe + export de l'état complet à chaque tour. Cela donne un oracle exact pour le moteur et permet de valider l'algorithme `rand` de §4.1.

### Phase 1 — Moteur autonome compilable (M)
1. Créer `engine/` avec `simul.cpp`/`simul.h` et la couche `compat/` (§3).
2. Introduire `IVueSimulation` (§4.3) et retirer les `#include "f_*.h"` du moteur.
3. Neutraliser les spécificités Borland (`#pragma warn`, `#pragma package`, `dstring.h`, `systdate.h`) ; macros `min`/`max` confinées pour ne pas entrer en conflit avec la STL.
4. Compiler en `-Wall` avec Clang (macOS), GCC (Linux) et MSVC (Windows). Ne corriger que ce qui empêche la compilation ; chaque modification annotée `// portage`.

**Fini quand** : `cvengine` compile sur les 3 OS et un petit exécutable console charge une carte, simule 100 tours et sauvegarde.

### Phase 2 — Filet de sécurité : tests du moteur (M)
1. **Round-trip** : pour les 6 cartes de `maps/`, `Charge` → `Sauve` → comparaison octet par octet. Attention, le chargement élimine les « feux fantômes » (v5.1.1) : un écart éventuel doit être expliqué, puis figé.
2. **Golden master** : graine fixe, N tours sur chaque carte, export de l'état (véhicules, piétons, feux, bus, trams, taxis, vehlibs, parkings, statistiques, épidémie) → fichiers de référence versionnés. Toute modification ultérieure du moteur doit les laisser identiques.
3. Même chose pour la génération aléatoire de réseau et pour « Copier vers… » (`PrepareCopie`).
4. `Verifie()` sur chaque carte → aucun message.
5. Si l'oracle instrumenté de la Phase 0 existe : comparer ses exports à ceux du port. **C'est la preuve que la logique est intacte.**
6. CI GitHub Actions (matrice 3 OS) qui exécute ces tests dès cette phase.

### Phase 3 — Ressources graphiques (S)
1. `tools/extrait_images.py` : parcourir les `.dfm` (`Image.Data`, `Glyph.Data`, `Picture.Data`) et extraire les PNG/BMP avec leur nom de collection (`Boutons\B000`, `Simulation\S000`…).
2. Générer la table d'index `VirtualImageList8x8` / `16x16` / `Glyphes` → fichier, pour que les constantes `IDB_*` de `f_simul.cpp` restent valables sans changement.
3. Convertir les BMP de `TBitBtn`/`TSpeedButton` (transparence = pixel en bas à gauche) en PNG avec canal alpha.
4. Contrôle croisé avec `res/png/`. Générer `cvw.qrc`, l'icône `.ico` (Windows) et `.icns` (macOS) depuis `res/ico/`.

### Phase 4 — Grille de simulation (M/L)
1. Widget `GrilleSimulation` (`QAbstractScrollArea`) qui reproduit `TDrawGrid` : cases 16×16, case courante (`Col`/`Row`), sélection rectangulaire (`goRangeSelect`), navigation clavier, clic, double-clic, clic droit (popup selon le mode), défilement.
2. Porter `DrawGridSimulDrawCell` presque tel quel : `VirtualImageList8x8->Draw(Canvas, x, y, i)` → `Sprites8x8.Draw(Canvas, x, y, i)` avec une petite classe `Canvas` imitant l'API VCL (`Pen`, `Brush`, `MoveTo`/`LineTo`, `FillRect`, `TextOut`) au-dessus de `QPainter`.
3. `RedessineCase` → `viewport()->update(rectCase)`.
4. HiDPI : coordonnées logiques en 16 px, mise à l'échelle **sans lissage** pour garder le pixel-art net à 150/200 %. Pas de zoom (retiré dans l'original en v5.4).

### Phase 5 — Fiche principale `TfrmSimulation` (L)
1. Menus, 100 actions, 7 barres d'outils, 13 popups, barre d'état, timers — même libellés, raccourcis, icônes, états cochés/grisés.
2. `TimerSimul` → `QTimer`, même intervalle (dérivé de `NbToursParSeconde`), même garde `TourEnCours`, même logique pause/arrêt.
3. Ouvrir/Enregistrer → `QFileDialog` (`*.cvw`), dossier par défaut équivalent à `stRepLocalAppData()` (sous Windows, **le même chemin**, pour retrouver les cartes déjà installées).
4. Ligne de commande conservée : `cvw fichier.cvw` et `cvw !` ; sous macOS, ouverture depuis le Finder via `QFileOpenEvent`.
5. Titre de fenêtre (`asFrmCaptionCV`), bilan de fin, messages : textes identiques.

### Phase 6 — Les 32 fiches secondaires (L, parallélisable)
1. `tools/dfm2ui.py` : conversion `.dfm` → `.ui` en conservant géométrie absolue (`Left/Top/Width/Height`), `Caption`, `Hint`, `TabOrder`, `Enabled`/`Visible`, bornes et valeurs des spins, éléments des `TRadioGroup`, polices, couleurs, boutons `Default`/`Cancel`, `ModalResult` ; production de la liste des gestionnaires d'événements à connecter.
2. Porter le code de chaque fiche avec la table de correspondance (§6).
3. Ordre conseillé, du plus simple au plus couplé :
   À propos → Dimensions zone → Génération → Épidémie → Nature → Paramètres (simulation, bus, tram, taxi, vehlib) → Définition voie / voie piétons → Feux / feux piétons → Carrefours → Distances (6 fiches à `TStringGrid`) → Infos (6 fiches non modales rafraîchies à chaque tour) → Statistiques (dessins `TPaintBox`) → Recherche.
4. Respecter la modalité (12 `ShowModal`, les autres non modales).
5. Pour chaque fiche : capture côte à côte original/port et liste de contrôle. Seule tolérance : ajustements de quelques pixels liés aux métriques de police (MS Sans Serif / Tahoma absentes sous macOS/Linux).

### Phase 7 — Services système (S)
- `AffichePageHtml` (registre + `CreateProcess`) → `QDesktopServices::openUrl` ; vérifier que l'ancre `#…` survit avec `file://` sur chaque OS, sinon visionneuse intégrée (`QTextBrowser`).
- Aide contextuelle F1 (`HelpContext` → ancre, table `caCorresp` conservée), index de l'aide, tutoriel.
- `ShellExecute` (À propos) → `openUrl`.
- Journal `cvw.log` au même emplacement.

### Phase 8 — Build, CI et distribution (M)
- `CMakePresets.json` ; GitHub Actions : `windows-latest` (MSVC), `macos-latest`, `ubuntu-latest` + tests.
- **Windows** : `windeployqt` + script Inno Setup existant adapté (`cvw.exe` + DLL/plugins Qt, association `.cvw` et cartes dans `{localappdata}` conservées). Qt 6 n'est plus distribué en 32 bits : abandon de `cvw.32.exe` (Windows 10/11 x64, ARM64 possible).
- **macOS** : `macdeployqt` → `Centre-Ville.app` + `.dmg`, association `.cvw` via `Info.plist`. Signature et notarisation nécessitent un compte Apple Developer, sinon avertissement Gatekeeper.
- **Linux** : AppImage (`linuxdeploy-plugin-qt`), fichier `.desktop` + type MIME pour `.cvw`.
- `build-installer.yml` compile depuis les sources au lieu d'exiger un `bin/` préexistant ; mise à jour du README.

### Phase 9 — Recette et bascule (M)
- Checklist complète : chaque menu, action, raccourci et fiche ; ouverture et simulation complète des 6 cartes ; épidémie ; mode « sanglant » et résurrection ; génération aléatoire ; « Copier vers… » ; vérification d'intégrité.
- **Interopérabilité** : les fichiers enregistrés par le port s'ouvrent dans l'original, et inversement.
- Bilan de fin de simulation identique à l'original à graine égale (si oracle instrumenté).
- Ensuite seulement : archiver `src/` VCL, taguer la nouvelle version.

---

## 6. Correspondance VCL → Qt

| VCL | Qt |
|---|---|
| `TForm` (principale / fiches) | `QMainWindow` / `QDialog` |
| `TMainMenu`, `TPopupMenu`, `TMenuItem` | `QMenuBar`, `QMenu`, `QAction` |
| `TActionList`, `TAction` | `QAction` (+ propriété `categorie`) |
| `TToolBar`, `TToolButton` | `QToolBar`, `QToolButton` |
| `TSpeedButton` (`GroupIndex`) | `QToolButton` checkable + `QButtonGroup` |
| `TButton`, `TBitBtn` | `QPushButton` (+ icône) |
| `TLabel` | `QLabel` |
| `TCSpinEdit` | `QAbstractSpinBox` sans type de valeur : le texte fait foi, bornes de `CSPIN.pas` (`app/vcl/controles.cpp`) |
| `TCheckBox`, `TRadioButton` | `QCheckBox`, `QRadioButton` (signal `toggled`) |
| `TGroupBox`, `TRadioGroup` | `QGroupBox` (+ `QButtonGroup` pour `ItemIndex`) |
| `TStringGrid` | widget maison sur `QAbstractScrollArea` (`app/vcl/grilletextes.cpp`, dessin de la VCL) |
| `TDrawGrid` | `GrilleSimulation` (widget maison, Phase 4) |
| `TPaintBox` | widget maison avec `paintEvent` |
| `TShape` | widget maison (rectangle/cercle) |
| `TImage` | `QLabel` avec `QPixmap` |
| `TPanel`, `TBevel` | `QFrame` |
| `TTrackBar` | `QSlider` |
| `TTabControl` | `QTabBar` |
| `TStatusBar` (panneaux) | `QStatusBar` + `QLabel` permanents |
| `TTimer` | `QTimer` |
| `TOpenDialog`, `TSaveDialog` | `QFileDialog` |
| `TImageCollection`, `TVirtualImageList` | `QPixmap` en ressources `.qrc` + table d'index |
| `MessageBoxW`, `MessageDlg` | `QMessageBox` |
| `TRegistry`, `GetEnvironmentVariable` | `QSettings`, `QStandardPaths` |
| `ShellExecute`, `CreateProcess` (navigateur) | `QDesktopServices::openUrl` |
| `AnsiString`, `Format`, `IntToStr` | `QString` dans l'UI ; shim `compat/chaine.h` dans le moteur |

---

## 7. Risques

| Risque | Parade |
|---|---|
| Logique modifiée par inadvertance lors du découplage | Golden master (Phase 2) avant toute refactorisation ; oracle instrumenté (Phase 0). |
| Séquence aléatoire différente | LCG Borland réimplémenté et vérifié contre l'original. |
| Incompatibilité des fichiers `.cvw` | `static_assert` sur les tailles, `signed char`, tests round-trip, test d'interopérabilité avec l'original. |
| Événements implicites VCL non reproduits (`OnChange`, `OnClick` déclenchés par le code) | Règles du §4.4 appliquées systématiquement ; recette fiche par fiche. |
| Rendu des polices différent sous macOS/Linux | Géométrie absolue conservée + revue visuelle par fiche. |
| Vitesse de simulation légèrement différente (résolution des timers selon l'OS) | Sans effet sur la logique (simulation tour par tour) ; seule la vitesse maximale perçue peut varier. |
| Volume de `f_simul.dfm` (640 Ko) et `f_simul.cpp` | Conversion outillée (`dfm2ui.py`, `extrait_images.py`) plutôt que manuelle. |

---

## 8. Premier sprint concret

1. `CMakeLists.txt` + `engine/` compilable sur macOS avec la couche `compat/`.
2. `IVueSimulation` + `VueNulle`, suppression des `#include "f_*.h"` du moteur.
3. Tests round-trip sur les 6 cartes.
4. Golden master : 500 tours × 6 cartes, graine fixe.
5. `tools/extrait_images.py` et vérification des index `IDB_*`.
6. Workflow CI « tests moteur » sur les 3 OS.
