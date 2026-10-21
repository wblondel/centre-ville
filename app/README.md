# `app/` : application Qt 6 `cvw`

Phases 4 et 5 de `PLAN_PORTAGE.md` (sprint 2) : la grille de simulation et la
fiche principale `TfrmSimulation` sont portées, ainsi que les fiches
Dimensions (`f_dimzone`) et Génération (`f_genres`) qu'utilise « Nouveau ».
Phase 6, lot 1 (paramètres et définitions) : les 13 fiches `f_propos`,
`f_epidemie`, `f_defn`, `f_prmsim`, `f_prmbus`, `f_prmtram`, `f_prmtaxi`,
`f_prmvehlib`, `f_defv`, `f_defvp`, `f_deff`, `f_deffp` et `f_carfours` sont
portées. Les 17 autres fiches secondaires (distances, infos, statistiques,
recherche) sont créées et affichées depuis les `.ui` générés, mais sans leur
code (lots 2 et 3).
Comme pour le moteur, **on porte, on ne réécrit pas** : les fiches portées
sont des copies de `src/f_*.*` (CRLF -> LF, `__fastcall` retiré), où chaque
autre modification est annotée `// portage`. Pour lire le portage ligne à
ligne :

```bash
git diff --no-index --word-diff src/f_simul.cpp app/f_simul.cpp
git diff --no-index --word-diff src/f_prmsim.cpp app/f_prmsim.cpp
```

## Construction et lancement

Qt 6.5 ou plus récent (CI : Qt 6.8 LTS ; Homebrew : `brew install qt`).

```bash
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
```

`CV_APP` vaut `AUTO` par défaut (l'application est compilée si Qt 6 est
trouvé), `ON` pour exiger Qt, `OFF` pour ne compiler que le moteur.

```bash
# macOS (paquet .app, avec l'association des fichiers .cvw pour le Finder)
build/dev/app/cvw.app/Contents/MacOS/cvw maps/Démo.cvw
# Linux / Windows
build/dev/app/cvw maps/Démo.cvw
```

`cvw !` active le journal `cvw.log` (écrit à la fermeture), comme l'original.
Les fichiers vont par défaut dans le dossier équivalent à `%LOCALAPPDATA%\Patquoi.fr\Centre-Ville\`
(`QStandardPaths::GenericDataLocation` + `Patquoi.fr/Centre-Ville` : le même
dossier sous Windows, `~/Library/Application Support/...` sous macOS,
`~/.local/share/...` sous Linux).

## Organisation

| Fichier | Rôle |
|---|---|
| `main.cpp`, `application.*` | Port de `cvw.cpp` : fiches créées au démarrage dans le même ordre ; `frmSimulation` (Visible = True dans le `.dfm`) est affichée avant la création des autres, comme dans la VCL. Ouverture depuis le Finder (`QFileOpenEvent`). |
| `f_simul.*` | Port de la fiche principale. `VueSimulation` y implémente `IVueSimulation` (les accès du moteur à l'interface). |
| `f_dimzone.*`, `f_genres.*` | Portées. `Genere(rand()%NbX, rand()%NbY, ...)` est séquencé explicitement x, puis y, puis la direction, comme le binaire d'origine. |
| `f_propos`, `f_epidemie`, `f_defn`, `f_prm*` (5), `f_defv`, `f_defvp`, `f_deff`, `f_deffp`, `f_carfours` | Portées (lot 1). `ChargeFiche()` remplace le chargement du `.dfm` ; `ShellExecute` (adresse, courriel) -> `QDesktopServices::openUrl` ; `MessageBoxW` (Licence) -> `AfficheMessage` ; `((TComponent*)Sender)->Tag`/`Name`/`ClassName()` -> `TagDe`/`NomDe`/`NomClasseDe` ; `__except` -> `catch(...)`. `f_carfours` appelle `IncrementeNombre<feu>`, instanciée explicitement dans `engine/simul.cpp`. |
| `f_*.*` (17 autres) | En-têtes portés ; fiches non portées : `.ui` généré sans gestionnaires, méthodes appelées par les autres fiches sans effet. |
| `base.*` | Port de `src/base.*` : `AfficheMessage` (`QMessageBox`), `MessageDlg`, `stRepLocalAppData`, journal, aide ; `min`/`max`. |
| `grille/GrilleSimulation.*` | Remplace le `TDrawGrid` : voir l'en-tête pour le comportement reproduit. |
| `vcl/` | Sous-ensemble de la VCL au-dessus de Qt, pour garder le code des fiches tel quel. |
| `ui/` | GÉNÉRÉ par `tools/dfm2ui.py` : `.ui` des 33 fiches et `<fiche>_composants.h`. |
| `ressources/` | GÉNÉRÉ par `tools/extrait_images.py` : images des `.dfm` et index des `TVirtualImageList`. |

### Couche `vcl/`

- `proprietes.h` : `Propriete<T>` reproduit les `__property` (`Checked^=true`,
  `Caption=Caption+"*"`, `Panels->Items[0]->Text=...`).
- `controles.*` : les contrôles portent le nom VCL (`TLabel`, `TCSpinEdit`,
  `TCheckBox`...) et dérivent du widget Qt le plus proche ; uic les instancie
  (widgets « promus » dans les `.ui`). Événements implicites du §4.4 :
  `Value=` → `OnChange`, `Checked=` → `OnClick` (`toggled`), `ItemIndex=` →
  `OnClick`...
- `actions.*` : `TAction` (objet non visuel), `TMenuItem` (un élément de menu
  par composant, même lié à une action), lien d'action de la VCL (un client ne
  suit l'action que s'il avait la même valeur avant le changement ; `Action=`
  recopie tout), `TAction::Execute` sans effet si l'action est désactivée.
- Raccourcis : la VCL choisit selon le contexte (menu contextuel du contrôle
  actif, puis menu principal, puis listes d'actions) ; Qt refuserait les
  doublons (Ctrl+B = « Passage de bus » en édition, « Détail sur un bus » en
  simulation). `RepartiteurRaccourcis` réclame la touche
  (`ShortcutOverride`) et reproduit cet ordre. Les chiffres et `VK_OEM_5` sont
  repérés par leur position physique (AZERTY compris).
- `fiche.*` : `TForm` (`TFormT<QDialog>`) et fiche principale
  (`TFormT<QMainWindow>`) : `ShowModal`/`ModalResult`, `OnCloseQuery`,
  `KeyPreview`, Échap = bouton `Cancel`, `Align`, liaison des membres aux
  objets créés par uic (X-macros `COMPOSANTS_<fiche>`/`EVENEMENTS_<fiche>`).
  `ShowModal` a sa propre boucle, que seul `ModalResult` termine (comme la
  VCL) : `f_prmsim` se cache le temps d'une fiche fille modale puis se
  réaffiche, ce que `QDialog::exec` ne permet pas (il s'arrête au `Hide`).
  Chaque réaffichage repasse par `OnShow`, comme dans la VCL. `BorderIcons`
  (boutons de la barre de titre : `biMinimize` pour `f_prmbus`, aucun pour
  `f_defn` et `f_dimzone`), `OnClick` de la fiche (clic hors des contrôles),
  `OnKeyPress` de la fiche quand aucun contrôle n'a le focus (`f_propos`),
  `ComponentCount`/`Components[i]` dans l'ordre du `.dfm` (`f_carfours`).
- Ajouts du lot 1 dans `controles.*` : `Font->Color`, `Color`, `Name` des
  contrôles ; `Glyph` des `TBitBtn`/`TSpeedButton` (`NumGlyphs` : 2e image
  pour l'état désactivé ; `GetBitmap(..., Glyph)` et `Glyph=...` redessinent
  le bouton, comme `TBitmap::OnChange`) ; `TSpeedButton` (jamais le focus,
  `Down` exclusif par `GroupIndex` avec `AllowAllUp`, `Click()` = `OnClick`
  seul) ; `TShape` repeint quand `Brush`/`Pen` changent ; `TLabel`/`TImage` :
  `OnClick` au relâchement, sans le transmettre à la fiche ; cases à cocher et
  boutons radio sensibles sur toute leur largeur (Qt : case et texte
  seulement). `Repaint()` pendant le dessin d'un autre contrôle devient
  `update()` : `DrawGridSimulDrawCell` repeint les barres d'outils dès qu'une
  sélection de plusieurs cases apparaît, et Qt interdit ce dessin imbriqué
  (plantage constaté par le test d'interface avant cette correction). `vcl.*` : `StrToInt`, `FormatFloat`,
  `Screen->PixelsPerInch` (96 × facteur d'échelle de l'écran principal),
  `UnicodeString`. `proprietes.h` : `(enum)Propriete<int>`.
- `graphiques.*` : `TColor`, `TRect` (bords droit et bas exclus), `TCanvas`
  sur `QPainter` ; `listeimages.*` : `TVirtualImageList` (indices `IDB_*`
  inchangés), images agrandies sans lissage en HiDPI.

## Tests

`tests/app/test_interface.cpp` (CTest `interface.demo`, plateforme
`offscreen`) lance l'interface comme `cvw <copie de Démo.cvw>` et la pilote :
raccourcis selon le mode, Marche / Interruption / Arrêt par la barre d'outils
et les menus, barre d'état, bilan (séparateur décimal `,`), puis enregistrer →
rouvrir (comme depuis le Finder) → réenregistrer : fichier identique, et égal à
la conversion de référence du format (`tests/support/format_reference`) ;
enfin Nouveau → Dimensions → Génération à graine fixe : même réseau que le
golden master `tests/golden/generation/defaut_12345.txt`.
Lot 1 des fiches secondaires, chacune sur une copie neuve de Démo et ouverte
par son menu, son bouton ou son action : un paramètre modifié et validé,
puis son effet sur le moteur (`cv->...`), et Annuler (ou Échap) sans effet ;
écran « À propos » du démarrage (animé, fermé seul) et du menu (feu vert,
orange, rouge ; Échap = Licence) ; Nature sur une sélection ; priorités et
feux, priorités et feux de piétons (trottoirs et passage piétons posés par
les actions d'édition) ; Paramètres et ses quatre fiches filles modales
(arrêts de bus et de tram, places de taxi et de vehlib posés par les actions
d'édition) ; Carrefours 4x4 à feux. Paramètres et Carrefours : enregistrer
puis rouvrir conserve les valeurs. Les fiches modales sont pilotées par
étapes (`QuandModale`).
`CV_DOSSIER_CAPTURES=<dossier>` enregistre les captures de la fenêtre
principale (Démo au tour 0 et au tour 200) et des fiches (`fiche_*.png`).

## Écarts connus avec l'original

Comportement :

- **Fiches secondaires** : les 17 fiches des lots 2 et 3 (Distances, Infos,
  Statistiques, Composition, Attente/trafic, Rechercher) sont affichées
  depuis le `.ui` généré, sans leur code : elles restent inertes.
- **Paramètres et fiches filles** : comme dans l'original, ouvrir une fiche
  fille (bus, tram, taxis, vehlibs) cache Paramètres puis la réaffiche, ce
  qui repasse par `FormShow` et recharge les valeurs du moteur (les saisies
  non appliquées sont perdues, comportement d'origine conservé). Seul écart :
  sous macOS, la console affiche alors « modalSession has been exited
  prematurely », sans effet.
- **`TCSpinEdit`** : `MinValue`/`MaxValue` ramènent tout de suite la valeur
  dans les bornes (`QSpinBox`), et la saisie au clavier est bornée pendant la
  frappe ; la VCL ne bornait qu'à l'affectation de `Value` et en quittant le
  champ.
- **Licence, adresse, courriel** (« À propos ») : boîte `AfficheMessage`
  (sans titre sous macOS) ; l'adresse et le courriel s'ouvrent avec le
  navigateur et la messagerie par défaut (`QDesktopServices`).
- **HiDPI** : `Screen->PixelsPerInch` vaut 96 × le facteur d'échelle de
  l'écran principal (« HDPI x2,0 » sur un écran Retina, comme Windows à
  200 %) ; l'agrandissement manuel de `f_defn` (v5.4) est neutralisé, Qt
  mettant déjà la fiche à l'échelle.
- **Raccourcis sous macOS** : « Ctrl » de la VCL est affiché ⌘ (convention de
  Qt) ; la touche Control physique est aussi acceptée. ⌘Q et ⌘H restent au
  système (Quitter, Masquer) : Carrefours (Ctrl+Q) et Rechercher (Ctrl+H) se
  font avec Control. Ctrl+flèches et Ctrl+Espace, pris par Mission Control et
  le changement de clavier avec Control, se font avec ⌘.
- **Ctrl+VK_OEM_5** (Arbre) : affiché Ctrl+\ ; c'est la touche physique qui
  compte (à gauche d'Entrée en ISO, `*` en AZERTY), comme sous Windows.
- **Boîtes de message** : sous macOS, `QMessageBox` est une alerte native,
  sans titre (« Bilan », « Confirmation »...) et dont la touche Échap suit les
  règles du système.
- **Barre d'état pendant les calculs** : les textes intermédiaires de Marche
  (« Calcul des distances... ») peuvent ne pas s'afficher sous macOS, où
  `repaint()` n'est présenté qu'au retour dans la boucle d'événements.
- **Vitesse** : `QTimer` a la même période que `TimerSimul`, mais la résolution
  des minuteries dépend du système : la vitesse maximale perçue peut varier
  (sans effet sur la simulation, tour par tour).
- **Fichiers** : filtre « Tous fichiers » en `*` au lieu de `*.*`.

Visuel :

- **Polices** : MS Sans Serif et Small Fonts n'existent pas hors de Windows :
  police système et Verdana/Tahoma à la place (barre d'état, numéros d'arrêts
  sur la carte, fiches) ; les tailles en pixels de la VCL sont conservées.
- **Menu principal** : dans la barre de menus du système sous macOS (hors de
  la fenêtre, qui est donc moins haute) ; icônes et raccourcis rendus par le
  système.
- **Boutons des barres d'outils** : positions et tailles de la VCL (23 × 22,
  deux rangées), mais rendu du style Qt ; icônes désactivées grisées par Qt.
- **Barre d'état** : panneaux aux largeurs de la VCL, séparés par un trait au
  lieu du relief de Windows ; les bulles d'aide des menus natifs de macOS ne
  s'y affichent pas.
- **Sélection** : couleur de sélection du système (accent de macOS) pour
  `clHighlight` ; barres de défilement superposées sous macOS.
- **HiDPI** : images agrandies au plus proche voisin ; aux facteurs non entiers
  (125 %, 150 %), certains pixels sont doublés et d'autres non.
- **Thème sombre** (macOS) : les fiches suivent l'apparence du système, sauf
  les couleurs fixées dans les `.dfm` (fond noir d'« À propos », sarcelle des
  priorités, couleurs des lignes de bus et de tram...).
- **Boutons** (`TButton`/`TBitBtn`) : hauteur et rendu du style du système
  (sous macOS, boutons arrondis plus bas que les 25 pixels de la VCL).
- **Glyphes désactivés** des `TSpeedButton` à une seule image (Carrefours) :
  calculés par le style Qt (grisés, ou simplement atténués sous macOS) au lieu
  du relief gris de la VCL ; les glyphes à deux images (`NumGlyphs = 2`)
  utilisent bien la seconde.
- **Groupes désactivés** (`TGroupBox`, `TRadioGroup`) : Qt grise aussi leur
  contenu ; la VCL rendait seulement les contrôles inaccessibles.
- **Textes longs** : avec les polices de remplacement, certains libellés à
  largeur fixe débordent ou sont coupés (texte d'« À propos », libellés de
  Paramètres, des lignes de bus et de tram...).
