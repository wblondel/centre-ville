# `tests/` : tests du moteur (phase 2 de `PLAN_PORTAGE.md`)

```bash
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
```

Catch2 3.8.1 est utilisé s'il est installé, sinon téléchargé par `FetchContent`.

| CTest | Étiquette Catch2 | Contenu |
|---|---|---|
| `moteur.compat` | `[compat]` | Couche `engine/compat` : `AnsiString`, `Format`, LCG Borland (comportement vérifié sur le binaire d'origine), fichiers (dont le tampon), `TDateTime`. |
| `moteur.cvw` | `[cvw]` | Round-trip `Charge` → `Sauve` octet par octet sur les 6 cartes et les 3 cartes du tutoriel, `Verifie()`, `VerifieReseau()`, feux fantômes, parkings indéfinis, version future. |
| `moteur.simulation` | `[simulation]` | Golden master : 500 tours × 6 cartes × 2 configurations, à graine fixe. |
| `moteur.generation` | `[generation]` | Génération aléatoire de réseau à graine fixe (3 configurations). |
| `moteur.copie` | `[copie]` | « Copier vers… » (`PrepareCopie`, `CopieValide`, `Sauve`). |
| `moteur.corrections` | `[corrections]` | Bogues d'origine corrigés en 6.0.1 (`test_corrections.cpp`) : blocage des vehlibs (Géant, graine 123), descente des passagers des vehlibs (Géant, graine 12345, cohérence de `NbPersonnes` et des réservations à chaque tour), erreurs d'écriture, « Copier vers… », messages de `Verifie()`. Délai de 120 s : le moteur 6.0.0 ne rendait pas la main. |
| `moteur.memoire` | tous sauf `[compat]` et `[corrections]` | `cvtests_motif` : round-trip, vérification, golden master, génération et copie avec une mémoire remplie de `0xA5` au lieu d'être remise à zéro, et champs que l'original laissait indéterminés (`test_memoire.cpp`). |
| `cvsim.spaghettis_100_tours` | — | Critère de fin de la phase 1 : charger une carte, simuler 100 tours, sauvegarder. |

## Harnais (`support/harnais.*`)

Le moteur n'a pas de point d'entrée « démarrer la simulation » : la séquence
d'appels se trouve dans la fiche principale. Le harnais la reproduit fidèlement,
sans les effets d'affichage :

- `Ouvre()` : `ActionOuvrirExecute`, qui appelle d'abord `AppelleAuChaos()` ;
- `VerifieReseau()` : copie de `TfrmSimulation::VerifieReseau` (lignes 1279 à
  1509 de `f_simul.cpp`) ;
- `Demarre()` : `ActionMarcheExecute` au premier démarrage. L'enregistrement
  automatique qui précède la simulation n'y apparaît que par son effet sur le
  moteur, `AppelleAuChaos()` ;
- `Tick()` : `TimerSimulTimer`, qui s'arrête quand tous les véhicules sont arrivés.

L'interface est remplacée par `VueNulle` (`engine/vue.h`), dont les options
« sanglante », « épidémie » et « résurrection » se règlent directement.

## Golden master (`support/golden.*`, `golden/`)

`alea::FixeGraine(12345)` force la graine de `srand()` : les deux appels
d'`AppelleAuChaos()` (ouverture et démarrage) donnent `srand(12345)`. C'est ce
qu'il faudra reproduire dans l'**oracle instrumenté** (phase 0), en remplaçant
`srand((unsigned)time(&t))` par `srand(12345)` dans `AppelleAuChaos()`.

Deux configurations :

- `standard` ;
- `epidemie` : épidémie, mode sanglant et résurrection demandée au tour 250.

Chaque fichier `golden/<config>/<carte>.txt` contient, pour chaque tour : l'état
du générateur aléatoire, des compteurs, et une empreinte 64 bits repliée sur 32
bits de chaque partie de l'état complet (`support/etat.*`). Ces parties sont :
centre_ville et statistiques, feux, feux piétons, parkings, véhicules, bus,
trams, taxis, places de parking, vehlibs, piétons avec leurs infections, et
cases avec attente et trafic. Le fichier se termine par l'empreinte des
distances, celle du détail attente/trafic au dernier tour et les messages
affichés.

Pour diagnostiquer un écart, chercher la première ligne qui diffère (le test
l'affiche) puis exporter l'état complet en texte avec `cvsim … --etat` et le
comparer avec celui obtenu avant la modification :

```bash
build/dev/tests/cvsim maps/Géant.cvw --graine 12345 --tours 120 --epidemie --sanglante --resurrection 250 --etat
```

Régénérer les fichiers de référence, **uniquement après avoir validé un
changement voulu** (le portage ne doit pas en produire) :

```bash
build/dev/tests/cvsim --golden maps tests/golden
```

Cette commande régénère `golden/standard`, `golden/epidemie`,
`golden/generation` et `golden/copie`. Les fichiers `golden/roundtrip` et
`golden/verifie` sont écrits à la main : ils expliquent les écarts figés.

## Déterminisme

- `support/allocation.cpp` remplace `operator new` par une version qui remet la
  mémoire à zéro, car le moteur d'origine lisait quelques champs non
  initialisés (voir `engine/README.md`). Depuis la 6.0.1, le moteur les
  initialise : `cvtests_motif` (`moteur.memoire`) refait les tests avec
  `support/allocation_motif.cpp`, qui remplit au contraire la mémoire de
  `0xA5`, et doit obtenir les mêmes résultats.
- Le golden master est identique avec Apple Clang (Debug, Release, ASan+UBSan)
  et avec GCC 12 sous Linux ARM64 (où `char` est non signé). Windows (MSVC) est
  couvert par la CI.

## Spécification du format `.cvw` (`support/format_reference.*`)

Lecture et réécriture d'un `.cvw` (versions 0x300 à 0x530) champ par champ,
indépendamment du moteur. Le test de round-trip vérifie trois choses :

- au format courant, la spécification reproduit le fichier à l'octet près ;
- le moteur produit exactement la conversion de référence, aux écarts listés
  près dans `golden/roundtrip/` (aucun pour les 6 cartes livrées ni pour les 3
  cartes du tutoriel) ;
- relire puis réenregistrer redonne le même fichier.
