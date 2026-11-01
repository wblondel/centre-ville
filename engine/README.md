# `engine/` : moteur de simulation `cvengine` (sans Qt)

Phases 1 et 2 de `PLAN_PORTAGE.md`. Le moteur de Centre-Ville 5.4.3.2 est
**recompilé, pas réécrit** : `simul.cpp` et `simul.h` sont des copies de
`src/simul.cpp` et `src/simul.h` (qui restent la référence, en lecture seule),
en fins de ligne LF. Chaque modification du portage (6.0.0) porte la mention
`// portage` ; chaque correction d'un bogue d'origine (6.0.1), la mention
`// v6.0.1`, comme l'auteur annotait ses versions (`// v5.3`...). Pour lire
les écarts avec l'original ligne à ligne :

```bash
git diff --no-index --ignore-cr-at-eol --word-diff src/simul.cpp engine/simul.cpp
```

## Construction et tests

```bash
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
```

Autres préréglages : `release`, `asan` et `asan-release` (AddressSanitizer +
UBSan, en Debug et en Release), `ci` (générateur par défaut, utilisé par
GitHub Actions) et `msvc` (Visual Studio).
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
- macros `min`/`max` retirées : le moteur ne les utilise pas, et elles entrent en conflit avec la STL (l'interface les utilise dans `f_infbus.cpp`, `f_inftram.cpp`, `f_infveh.cpp` et `f_simul.cpp` : elles sont redéfinies en fonctions dans `app/base.h`) ;
- les 19 énumérés `: char` passent en `: signed char`, car `char` est non signé sous Linux ARM64. Le moteur est en plus compilé en `-fsigned-char` ;
- `__fastcall` retiré de `EffectueTourSimulation` ;
- `friend class ExportEtat` dans `infection` : sert uniquement à l'export de l'état par les tests.

`simul.cpp` (numéros de ligne : ceux de `src/simul.cpp` ; ceux de
`engine/simul.cpp` sont décalés par les lignes ajoutées) :

- `#include <vcl.h>`, `#pragma hdrstop` et `#pragma package` neutralisés ;
- `base.h` et les 9 `f_*.h` remplacés par `vue.h`, `format_cvw.h`, `compat/fichier.h` et `compat/alea.h` ;
- `__fastcall` retiré ;
- **chaque accès à l'interface est remplacé un pour un**, dans le même ordre, par un appel à `vue->…`.
- instanciation explicite de `IncrementeNombre<feu>`, appelée par `app/f_carfours.cpp` (phase 6) : sans elle, l'optimiseur peut ne garder aucune copie appelable de l'instance implicite ;
- `voie::DefinitNature` teste `d<=ouest` avant de lire `PossDir[d]` : l'original lisait `PossDir[5]` hors du tableau, comportement indéfini que Clang exploite en `-O2` (la nature d'une sélection n'était jamais définie en Release). Le résultat est celui du binaire d'origine (sortie de boucle à `d=5`).

### Inventaire des accès à l'interface remplacés

Les numéros de ligne sont ceux de `src/simul.cpp`. Le code en commentaire n'a pas été touché (`Genere()`, lignes 6606-6609, 6628, 6653 et 6665-6667).

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
  `EConvertError`). Les séparateurs viennent du global `FormatSettings`
  (`DecimalSeparator`, `ThousandSeparator`), comme dans SysUtils : `.` et `,`
  par défaut (moteur et tests), `,` et espace insécable dans l'application,
  comme la VCL en français (sprint 2). Le résultat ne dépend pas de
  `LC_NUMERIC`. `TVarRec` accepte aussi `long double` (casts de
  `AfficheBilan`), et `ARRAYOFCONST` passe par `VersTVarRec`, que
  l'application surcharge pour ses propriétés (`ARRAYOFCONST((Grille->Col))`).
  Ces trois ajouts du sprint 2 sont annotés `// portage` ; le golden master est
  inchangé.
- **Fichiers** : poignées `int` strictement positives, `-1` en cas d'échec,
  `FileRead`/`FileWrite` renvoient le nombre d'octets, ou 0 en fin de fichier,
  ou -1. Les noms sont en UTF-8 (convertis en UTF-16 sous Windows). Un tampon
  évite un appel système par champ ; il ne change pas le comportement
  observable, sauf pour les erreurs d'écriture, qui ne sont connues qu'en le
  vidant : depuis la 6.0.1, après un échec, les écritures suivantes de la
  poignée échouent aussi, et `FileFlush` (absente de SysUtils) vide le tampon
  et dit si toutes les écritures ont abouti. `centre_ville::Sauve` l'appelle
  en dernier.
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

## Corrections de la 6.0.1 (annotées `// v6.0.1`)

La règle « on porte, on ne corrige pas » est levée à partir de la 6.0.1.
Chaque correction d'un bogue listé dans le `CHANGELOG.md` de la 6.0.0, ou
trouvé en 6.0.1, a son commit et un test qui échoue sur le moteur 6.0.0
(`tests/test_corrections.cpp`,
`tests/test_memoire.cpp`, `test_compat.cpp`, et deux tests de l'interface) ;
l'audit des lectures hors tableau s'appuie sur les sanitizers (plus bas).

- **Blocage des vehlibs** : `vehlib::ReservePlaceVehlibCible` tirait au
  hasard une place libre sans limite d'essais ; sans place libre, la
  simulation se figeait (Géant, 29 vehlibs pour 32 places : 10 graines sur 60
  avant le tour 2000, la graine 123 avant le tour 50). Elle renvoie
  désormais `false`, sans tirage, si aucune place autre que la place de
  départ n'est libre. Le piéton ne monte (`FaitMonterPieton`) et le vehlib
  ne part (`CalculeProchaineDirection`) qu'avec une place d'arrivée
  réservée ; sinon le vehlib reste en `svAttente` et réessaie au tour
  suivant. C'est l'intention de l'original : `NbPietonsQuiSontMontes` ne
  fait monter les piétons que « s'il y a des places libres dans la
  simulation » (test global `NbVehlibs<NbPlacesVehlib`, qui ne voyait pas
  qu'un vehlib qui part occupe encore sa place de départ), et un vehlib ne
  roule jamais sans destination. Quand une place est libre, les tirages sont
  ceux de l'original. Graines 1 à 80, 100 à 140 et 12345 : 2000 tours sur
  Géant.
- **Passagers des vehlibs** (trouvé en 6.0.1) : ils ne descendaient jamais.
  `vehlib::FaitDescendrePietons` cherchait la direction de sortie en testant
  `PietonPeutAller(d)` sur la case du vehlib, toujours fausse puisqu'il
  l'occupe, au lieu de la case voisine dans cette direction, sur laquelle il
  fait ensuite descendre le piéton ; et `voie::FaitDescendrePieton` ne
  remettait pas `pieton::NumVehlib` à zéro ni ne décrémentait
  `vehlib::NbPersonnes` (seulement pour les bus, trams et taxis). Un vehlib
  qui avait pris un passager repartait donc dès son arrivée, puisque
  `NbPietonsQuiSontMontes()` comptait ce passager, ce qui usait les places
  libres ; le piéton n'était plus jamais simulé. Les deux sont corrigés comme
  pour les taxis. À l'arrivée à la place réservée
  (`centre_ville::PlaceVehiculeVehlib`, inchangé), chaque passager descend
  sur la première case voisine de la place (nord, est, sud, ouest) où un
  piéton peut aller, un trottoir en pratique, ou à côté de celle-ci si elle
  est pleine, puis redevient un piéton simulé. Le vehlib attend ensuite à sa
  place d'arrivée, devenue sa place de départ, qu'un piéton le prenne. Si
  aucune case voisine ne peut accueillir de piéton, le passager reste à bord
  et le vehlib repart aussitôt vers une autre place libre, ou attend qu'une
  place se libère : c'est le cas que l'original prévoyait (« Des fois les
  piétons restent dans le vehlib... », `CalculeProchaineDirection`), comme
  le passager d'un bus qui « descendra au prochain arrêt ». Sur Géant
  (graines 1 à 80, 100 à 140 et 12345, 2000 tours, standard et épidémie +
  mode sanglant + résurrection) : 13 304 arrivées, 13 186 descentes, 587 fois
  un passager resté à bord (4 %) ; `NbPersonnes` et les réservations de
  places restent cohérents à chaque tour.
- **Distances après un réseau non connexe** (trouvé en 6.0.1) : quand Marche
  trouve le réseau non connexe, les distances restent allouées pour que les
  fiches les affichent. Si l'utilisateur répare alors le réseau,
  `voie::PrepareCalculDistances` et `~voie` les libéraient d'après `DirPoss` et
  les compteurs de `cv`, qui ont changé : `delete[]` hors du tableau, ou fuite.
  `voie` retient les dimensions de l'allocation (hors du format `.cvw`), et
  `centre_ville::LibereDistances`, appelée dès que l'application voit le réseau
  modifié, évite aux fiches de relire ces tableaux avec les nouvelles dimensions.
- **Erreurs d'écriture** : `voie::Sauve` contenait un `;` au lieu de `&&`,
  `centre_ville::Sauve` écrasait `Ok` par le résultat de `DepotBus.Sauve`, et
  surtout `FileWrite` renvoie -1 (vrai dans les chaînes de `&&`) en cas
  d'erreur. `centre_ville::Sauve` se termine par `FileFlush` : toute erreur
  d'écriture le fait échouer, et l'application affiche « Erreur de données
  dans le fichier ».
- **`Verifie*()`** : `VerifiePosition`, `VerifieDirection`, `VerifiePriorite`,
  `VerifieDirPoss`, `VerifieEtat`, `VerifieDmdVert` et `VerifiePsgRouge`
  reçoivent `asMsgErr` par référence. Une fois leurs messages rendus, chaque
  place libre du tableau des feux (feu supprimé, `x=y=-1`, que
  `NumeroFeuLibre` réutilise) était signalée : `centre_ville::Verifie` ne
  vérifie plus que les feux définis.
- **`depot_bus::Verifie`** : `Format("Dépot bus (%d,%d)", …)` au lieu de
  `Format("Dépot bus : ", …)`, sans spécificateur.
- **« Copier vers… »** : `PrepareCopie(FIN, …)` remet `NbYc` à zéro (et non
  le paramètre `NbY`) ; `COORDCOPIE` garde -1 pour une coordonnée non
  définie au lieu d'enregistrer `-1-Xc` (-2 pour une sélection en (1,1)),
  que la vérification d'intégrité signalait à la réouverture de la copie.
- **Champs non initialisés** : `centre_ville::NbVehlibs` (fichiers
  antérieurs à 0x530), `centre_ville::FeuP` (libéré par `IncrementeNombre`
  au premier feu de piétons), `centre_ville::DureeVert` et `DureeRouge`
  (nouveau réseau), `pieton::TourAttBus/TourAttTram/TourAttTaxi` valent 0
  (ou `NULL`), comme dans la 6.0.0 qui remettait la mémoire à zéro. Le test
  `moteur.memoire` (`cvtests_motif`) refait le round-trip, la vérification,
  le golden master, la génération et la copie avec une mémoire remplie de
  `0xA5` : le moteur ne dépend plus de la remise à zéro.

Golden master : `tests/golden/copie/*.txt` changent (`NbYc=0` après
« Copier vers… », empreinte de la copie réduite), ainsi que
`standard/Geant.txt` et `epidemie/Geant.txt` à partir du tour 7, quand un
vehlib arrive pour la première fois avec un passager : son passager
descend et il attend, au lieu de repartir en tirant une nouvelle place
d'arrivée. Géant est la seule carte livrée avec des vehlibs ; la simulation
des 5 autres cartes, la génération et `verifie/` sont inchangées.

### Audit des lectures hors tableau

Même motif que `voie::DefinitNature` (corrigé en 6.0.0) : une condition de
boucle qui utilise l'indice (tableau, appel, case voisine) avant le terme
qui le borne, ou un indice réutilisé après une boucle de recherche sortie à
la borne. Toutes les conditions `for`, `while` et `if` de `engine/` ont été
recensées par `tools/audit_bornes.py` (qui retrouve aussi `DefinitNature` dans
`src/simul.cpp`), et les boucles de recherche relues une à une. Cinq lisaient
hors du tableau à la borne ; la borne y est désormais testée d'abord, avec le
résultat de l'original :

| Fonction | Accès hors du tableau |
|---|---|
| `vehlib::FaitDescendrePietons` (`src/simul.cpp:1401`, `engine/simul.cpp:1404` en 6.0.0) | `PietonPeutAller(5)`, en sortie de boucle puis dans le `if` qui suit |
| `bus::NumeroArretSuivant`, `tram::NumeroArretSuivant` | `NumArret[NbArrets]`, hors du tableau pour une ligne de 9 arrêts si l'arrêt cherché n'y est pas |
| `voie::AjoutePieton` | lecture puis **écriture** de `NumPieton[2]` (sur le pointeur `DistParking` qui suit) si `NbPietons` ne correspond pas aux places occupées : renvoie désormais `false` |
| `voie::RetirePieton` | lecture de `NumPieton[2]` si le piéton n'est pas sur la case |

Écartés : `voie::NumDirPossSensVoieTram` compare `DirCrt=5` sans accéder à un
tableau ; les cases voisines (`V`, `V_`, `_V_`) sont prises modulo la taille
de la carte ; les autres conditions testent un drapeau ou une borne basse.
Dans l'interface, `app/f_simul.cpp` a le même motif (`PietonPeutAller` avant
`d<=ouest`, corrigé en 6.0.2). Hors de ce motif, le moteur fait confiance
aux valeurs lues dans le fichier : des directions possibles invalides
(`DirPoss` > 15) font lire hors de `NbDir` (par exemple dans `~voie()`) ;
`Verifie()` les signale, sans les corriger.

Aucune de ces lectures ne se produit sur les cartes livrées. Les 6 cartes de
`maps/` et les 3 de `docs/tutorial/` ont tourné sous ASan et UBSan (rapports
dans le journal, `UBSAN_OPTIONS=print_stacktrace=1`), en configuration
standard et en épidémie + mode sanglant + résurrection à mi-parcours, avec
`cvsim --graine G` :

| Moteur | Build | Graines | Tours | Simulations | Rapports |
|---|---|---|---|---|---|
| avant l'audit | `asan-release` | 1 à 30 | 1000 | 540 | aucun |
| 6.0.1 | `asan-release` | 1 à 40 | 2000 | 720 | aucun |
| 6.0.1 | `asan` (Debug) | 1 à 20 | 1000 | 360 | aucun |
| 6.0.1, passagers des vehlibs | `asan-release` | 1 à 40 | 2000 | 720 | aucun |
| 6.0.1, passagers des vehlibs | `asan` (Debug) | 1 à 20 | 1000 | 360 | aucun |

Un programme témoin du motif de `voie::AjoutePieton` est bien signalé par
UBSan (`index 2 out of bounds for type 'int[2]'`). `ctest --preset asan`
et `ctest --preset asan-release` (moteur) passent sans rapport depuis la 6.0.2,
`interface.demo` compris (fiche Taxi, `f_inftaxi.cpp:87`, qui l'arrêtait, corrigée). La CI
lance `ctest --preset asan` sous Linux, et sous macOS sur `main`, une erreur d'UBSan arrêtant le test.

## Corrections de la 6.0.3 (annotées `// v6.0.3`)

- **`centre_ville::Verifie`** : `NbMaxVehicules` divisait par
  `NbParkings*(NbParkings-1)` avant le test du nombre de parkings. Arrêt sur x86,
  valeur quelconque sur ARM (relevé par UBSan). Le message « Le nombre de parkings est
  incorrect » est désormais donné.

## Corrections de la 6.0.2 (annotées `// v6.0.2`)

- **`centre_ville::Charge`** : tous les `Charge()` lisent par `LitChamp`, qui n'accepte
  qu'un champ entier. `FileRead` renvoie -1 en cas d'erreur, vrai dans les chaînes de
  `&&` : un fichier illisible donnait un réseau vide, et un champ lu en partie (fichier
  coupé dans son dernier champ) comptait pour lu. `CreeZone` n'est plus appelée sur des
  dimensions mal lues. `Charge(-1)` échoue. `NombreCredible` refuse un nombre d'éléments
  négatif ou plus grand que le fichier (ou une zone sans case) avant `new[]`, qui n'échoue
  pas forcément sur un nombre énorme. Les valeurs des champs (`DirPoss`, numéros...) restent
  à la charge de `Verifie()`.
- **`centre_ville::RetablitVoisinages`** : `dx` et `dy`, globales, suivent le dernier
  réseau créé ; l'interface la rappelle quand elle revient au réseau affiché après un
  chargement en échec.
- **`~centre_ville`** libère `FeuP`, `Bus` et `Tram`.
- **`taxi::Defini`**, ajoutée pour Rechercher, comme celle des autres véhicules.
- **`compat/fichier`** : `FileCreateRemplacement`, `FileCommitRemplacement` et
  `FileAbandonRemplacement` remplacent un fichier en un seul geste (voir leur en-tête).
  Sans `rand` : le nom du fichier temporaire ne dépend que du processus et d'un compteur.

Rien de tout cela ne change le golden master ni les octets écrits.

## Points d'attention (non corrigés)

- **Ordre d'évaluation des arguments** : `f_genres.cpp` appelle
  `cv->Genere(rand()%NbX, rand()%NbY, 1+rand()%NBDIR)`. L'ordre des trois
  `rand()` n'est pas spécifié en C++ ; le binaire d'origine les évalue de
  gauche à droite (voir « Vérifications sur le binaire d'origine »). Lors du portage de la fiche, il
  faudra donc séquencer explicitement x, puis y, puis la direction. Le moteur
  lui-même ne contient pas de tel cas.
- **Pile** : le calcul des distances (`voie::CalculeDistance*`) est récursif. Il
  demande entre 256 et 512 Ko de pile avec Clang, davantage en Debug MSVC ou
  sous ASan. L'original tournait avec la pile par défaut de 1 Mo. Les
  exécutables de test réservent 16 Mo (Windows) ou 64 Mo (macOS), l'application
  16 Mo (Windows et macOS). La récursion descend au plus d'un niveau par case, à
  environ 160 octets le niveau : 6,4 Mo pour une zone de 200x200 (`NBXMAX`,
  `NBYMAX`, v6.0.3), moins que les 8 Mo du fil principal sous Linux. Une carte
  plus grande, qui ne peut venir que d'un fichier, n'a pas cette garantie.
- **Carfours.cvw** : `Verifie()` signale « Le nombre de véhicules ne permet pas
  une simulation équilibrée (1000) » (1000 n'est pas multiple de 4×3). C'est
  le réseau d'origine qui est en cause, pas le portage : le message est figé
  dans `tests/golden/verifie/Carfours.txt`.
- **Constructeur inutilisé** : `pieton(int, int, direction, int)` n'appelle pas
  `Initialise()`.
