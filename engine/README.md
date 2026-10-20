# `engine/` : moteur de simulation `cvengine` (sans Qt)

Phases 1 et 2 de `PLAN_PORTAGE.md`. Le moteur de Centre-Ville 5.4.3.2 est
**recompilé, pas réécrit** : `simul.cpp` et `simul.h` sont des copies de
`src/simul.cpp` et `src/simul.h` (qui restent la référence, en lecture seule).
Chaque modification porte la mention `// portage` et **aucune ligne n'a été
ajoutée ni retirée** : les fichiers ont le même nombre de lignes que l'original,
avec les mêmes fins de ligne (CRLF). Pour lire le portage ligne à ligne :

```bash
git diff --no-index --word-diff src/simul.cpp engine/simul.cpp
```

## Construction et tests

```bash
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
```

Autres préréglages : `release`, `asan` (AddressSanitizer + UBSan), `ci`
(générateur par défaut, utilisé par GitHub Actions) et `msvc` (Visual Studio).
Les tests sont décrits dans `tests/README.md`.

## Fichiers

| Fichier | Rôle |
|---|---|
| `simul.cpp`, `simul.h` | Moteur d'origine, porté (voir ci-dessous). |
| `vue.h`, `vue.cpp` | Interface `IVueSimulation` entre le moteur et l'interface, pointeur global `vue`, implémentation `VueNulle`. |
| `format_cvw.h` | `static_assert` sur les tailles de tous les types et champs sérialisés dans les `.cvw`. |
| `compat/chaine.*` | Remplace `<dstring.h>` : sous-ensemble d'`AnsiString`, `Format(…, ARRAYOFCONST((…)))`, `IntToStr`. |
| `compat/fichier.*` | `FileOpen`, `FileCreate`, `FileRead`, `FileWrite`, `FileSeek`, `FileClose` avec la sémantique de SysUtils. |
| `compat/alea.*` | `rand()`/`srand()` de la RTL Borland, graine injectable. |
| `compat/temps.*` | Remplace `<systdate.h>` : `TDateTime`, `Time()`, `Now()`. |

## Modifications du moteur (toutes annotées `// portage`)

`simul.h` :

- `#pragma warn -pia` neutralisé, `<dstring.h>` et `<systdate.h>` remplacés par `compat/chaine.h` et `compat/temps.h` ;
- macros `min`/`max` retirées : le moteur ne les utilise pas, et elles entrent en conflit avec la STL (l'interface les utilise dans `f_infbus.cpp`, `f_inftram.cpp`, `f_infveh.cpp` et `f_simul.cpp` ; à reprendre lors du portage de ces fiches) ;
- les 19 énumérés `: char` passent en `: signed char`, car `char` est non signé sous Linux ARM64. Le moteur est en plus compilé en `-fsigned-char` ;
- `__fastcall` retiré de `EffectueTourSimulation` ;
- `friend class ExportEtat` dans `infection` : sert uniquement à l'export de l'état par les tests.

`simul.cpp` :

- `#include <vcl.h>`, `#pragma hdrstop` et `#pragma package` neutralisés ;
- `base.h` et les 9 `f_*.h` remplacés par `vue.h`, `format_cvw.h`, `compat/fichier.h` et `compat/alea.h` ;
- `__fastcall` retiré ;
- **chaque accès à l'interface est remplacé un pour un**, dans le même ordre, par un appel à `vue->…`.

### Inventaire des accès à l'interface remplacés

Les numéros de ligne sont identiques dans `src/simul.cpp` et `engine/simul.cpp`. Le code en commentaire n'a pas été touché (`Genere()`, lignes 6606-6609, 6628, 6653 et 6665-6667).

| Nb | Original | Remplacement | Lignes |
|---:|---|---|---|
| 24 | `frmSimulation->RedessineCase(x, y, e)` | `vue->RedessineCase(x, y, e)` | 514, 867, 1427, 1441, 1529, 1795, 2269, 2867, 2886, 2903, 2910, 3226, 3909, 5774, 5788, 5863, 5868, 5882, 5898, 6028, 6034, 6038, 6041, 6044 |
| 7 | `if (frmSimulation) frmSimulation->RedessineCase(…)` | `if (vue) vue->RedessineCase(…)` | 3234, 3245, 3254, 3260, 3266, 3589, 3603 |
| 6 | `frmSimulation->SimulationSanglante` | `vue->SimulationSanglante()` | 2673, 5418, 5490, 5524, 5553, 5588 |
| 2 | `frmSimulation->EpidemieActivee` | `vue->EpidemieActivee()` | 3196, 4239 |
| 1 | `frmSimulation->ActionEpidemie->Checked` | `vue->ActionEpidemieCochee()` | 5968 |
| 1 | `frmSimulation->ResurrectionDemandee` (lecture) | `vue->ResurrectionDemandee()` | 5956 |
| 1 | `frmSimulation->ResurrectionDemandee=false` | `vue->DefinitResurrectionDemandee(false)` | 5959 |
| 1 | `frmSimulation->SimulationModifiee=true` | `vue->DefinitSimulationModifiee(true)` | 3885 |
| 6 | `frmSimulation->StatusBar->Panels->Items[0]->Text=t` | `vue->AfficheStatut(0, t)` | 5797, 5807, 5817, 5827, 5836, 5845 |
| 6 | `frmSimulation->StatusBar->Panels->Items[3]->Text=t` | `vue->AfficheStatut(3, t)` | 5801, 5811, 5821, 5831, 5840, 5849 |
| 12 | `frmSimulation->StatusBar->Repaint()` | `vue->RepeintStatut()` | 5798, 5802, 5808, 5812, 5818, 5822, 5828, 5832, 5837, 5841, 5846, 5850 |
| 1 | `if (frmStatistiques->Visible) frmStatistiques->PaintBoxStats->Repaint();` | `if (vue->FicheVisible(fiStatistiques)) vue->RafraichitFiche(fiStatistiques);` | 6048 |
| 7 | `if (frmXxx->Visible) frmXxx->RafraichitInfos();` (StatsVehicules, InfosVehicule, InfosBus, InfosTram, InfosTaxi, InfosVehlib, InfosPieton) | `if (vue->FicheVisible(fiXxx)) vue->RafraichitFiche(fiXxx);` | 6049 à 6055 |
| 1 | `frmRecherche&&frmRecherche->Visible` | `vue->RechercheVisible()` | 6056 |
| 6 | `frmRecherche->RadioButtonXxx->Checked` | `vue->RechercheCochee(orXxx)` | 6058, 6060, 6062, 6064, 6066, 6068 |
| 6 | `frmRecherche->CSpinEditXxxChange(frmRecherche->CSpinEditXxx)` | `vue->RechercheActualise(orXxx)` | 6059, 6061, 6063, 6065, 6067, 6069 |
| 2 | `AfficheMessage(…)` (`base.cpp`) | `vue->AfficheMessage(…)` | 5693, 6592 |

Les types `type_message` et `type_reponse` (paramètres d'`AfficheMessage`) sont
déplacés de `src/base.h` vers `vue.h`. Dans l'application, `vue` jouera le
rôle des pointeurs globaux de fiches : il vaut `NULL` tant que l'interface n'est
pas créée, d'où les gardes `if (vue)` qui reproduisent les `if (frmSimulation)`.

## Couche `compat/`

- **`AnsiString`** encapsule un `std::string`. Les littéraux du dépôt sont en
  UTF-8 depuis la conversion de l'encodage, alors qu'ils étaient en Windows-1252
  dans l'original : `Length()` compte donc des octets. Le moteur ne dépend
  jamais de la longueur d'une chaîne accentuée.
- **`Format`** applique les règles de SysUtils : index `%0:`, `-`, largeur,
  précision, `*`, types `d u x e f g n s p`. Les arguments surnuméraires sont
  ignorés, un argument manquant ou d'un mauvais type lève une exception (comme
  `EConvertError`). Le séparateur décimal est `.` ; la VCL utilisait celui des
  paramètres régionaux (`,` en français). C'est sans effet sur le moteur, qui
  n'utilise que `%d` et `%s`, mais **à reprendre pour l'interface (phase 6)**.
- **Fichiers** : poignées `int` strictement positives, `-1` en cas d'échec,
  `FileRead`/`FileWrite` renvoient le nombre d'octets, ou 0 en fin de fichier,
  ou -1. Les noms sont en UTF-8 (convertis en UTF-16 sous Windows). Un tampon
  évite un appel système par champ ; il ne change pas le comportement
  observable.
- **`rand()`/`srand()`** : LCG de la RTL Embarcadero, conforme au binaire
  d'origine (voir la section suivante). Un pas : `graine = graine*0x15A4E35+1`
  sur 32 bits. `rand()` fait un pas puis renvoie les bits 30..16 (`RAND_MAX =
  32767`). `srand(g)` pose la graine **puis fait un pas**. L'état initial est
  celui de `_boot_srand` : 1 puis un pas (0x015A4E36). Ce sont des macros
  définies après les en-têtes système. `alea::FixeGraine(g)` force la graine
  utilisée par `srand()` : c'est l'équivalent de `srand(g)` à la place de
  `srand(time(…))` dans `AppelleAuChaos()`, sans toucher au moteur.
- **`TDateTime`** : nombre de jours en `double`. Le moteur ne s'en sert que pour
  chronométrer.

## Vérifications sur le binaire d'origine

Les deux exécutables de l'installeur officiel 5.4.3.2 (32 et 64 bits, compilés
avec la RTL Embarcadero) ont été désassemblés avec `llvm-objdump`. Ils se
comportent de la même façon.

| Point | Constat | Adresses (64 bits / 32 bits) |
|---|---|---|
| `rand()` | `graine = graine*0x15A4E35+1` (`imul` 32 bits), puis renvoie `(graine>>16) & 0x7FFF`. | `0x767ea0` / `0x758c14` |
| `srand(g)` | `graine = g`, mot haut de la graine = 0, **puis un pas** : `graine = graine*0x15A4E35+1`. | `0x767e50` / `0x758bd4` |
| État initial (`_boot_srand`) | `graine = 1`, puis un pas : `0x015A4E36` (22695478). | `0x767ee0` / — |
| `Genere(rand()%NbX, rand()%NbY, 1+rand()%NBDIR)` (`f_genres.cpp`) | Arguments évalués **de gauche à droite** : 1er tirage `% NbX` (x), 2e `% NbY` (y), 3e pour la direction. | `0x41544d` / `0x416d63` |

Conséquences pour le portage :

- `compat/alea` reproduit exactement ce comportement : le pas fait par
  `srand()` et l'état initial `_boot_srand`. Le premier `rand()` après
  `srand(1)` donne donc 130, deuxième valeur de la suite Turbo C.
- Le golden master a été régénéré après cette correction de fidélité. Il
  correspond désormais à un oracle instrumenté avec `srand(12345)` dans
  `AppelleAuChaos()`.
- Le portage de la fiche « Génération » devra séquencer explicitement x, puis
  y, puis la direction. Le golden master de la génération
  (`tests/support/golden.cpp`) le fait déjà.

## Points d'attention relevés pendant le portage (non corrigés)

- **Ordre d'évaluation des arguments** : `f_genres.cpp` appelle
  `cv->Genere(rand()%NbX, rand()%NbY, 1+rand()%NBDIR)`. L'ordre des trois
  `rand()` n'est pas spécifié en C++ ; le binaire d'origine les évalue de
  gauche à droite (voir la section suivante). Lors du portage de la fiche, il
  faudra donc séquencer explicitement x, puis y, puis la direction. Le moteur
  lui-même ne contient pas de tel cas.
- **Champs non initialisés** : `centre_ville::NbVehlibs` (fichiers < 0x530),
  `pieton::TourAttBus/TourAttTram/TourAttTaxi`, `centre_ville::FeuP` (sans feux
  piétons). Pour les fichiers < 0x530, l'original enregistre une valeur
  indéterminée de `NbVehlibs`. Les tests remettent la mémoire à zéro pour être
  déterministes (`tests/support/allocation.cpp`).
- **Pile** : le calcul des distances (`voie::CalculeDistance*`) est récursif. Il
  demande entre 256 et 512 Ko de pile avec Clang, davantage en Debug MSVC ou
  sous ASan. L'original tournait avec la pile par défaut de 1 Mo. Les
  exécutables de test réservent 16 Mo (Windows) ou 64 Mo (macOS) ;
  l'application devra prévoir une marge équivalente.
- **Carfours.cvw** : `Verifie()` signale « Le nombre de véhicules ne permet pas
  une simulation équilibrée (1000) » (1000 n'est pas multiple de 4×3). C'est
  le réseau d'origine qui est en cause, pas le portage : le message est figé
  dans `tests/golden/verifie/Carfours.txt`.
- Bogues d'origine conservés (règle d'or), à reporter dans `src/A_FAIRE.txt`
  après la bascule :
  - `Verifie*()` reçoivent `asMsgErr` par valeur : leurs messages sont perdus ;
  - `PrepareCopie(FIN, …)` remet à zéro le paramètre `NbY` au lieu de `NbYc` ;
  - `voie::Sauve` contient un `;` au lieu de `&&` : le résultat des dernières
    écritures est ignoré ;
  - `depot_bus::Verifie` utilise `Format("Dépot bus : ", …)` sans spécificateur.
