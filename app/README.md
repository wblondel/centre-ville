# `app/` : application Qt 6 `cvw`

Phases 4 et 5 de `PLAN_PORTAGE.md` (sprint 2) : la grille de simulation et la
fiche principale `TfrmSimulation` sont portées, ainsi que les fiches
Dimensions (`f_dimzone`) et Génération (`f_genres`) qu'utilise « Nouveau ».
Phase 6, lot 1 (paramètres et définitions) : les 13 fiches `f_propos`,
`f_epidemie`, `f_defn`, `f_prmsim`, `f_prmbus`, `f_prmtram`, `f_prmtaxi`,
`f_prmvehlib`, `f_defv`, `f_defvp`, `f_deff`, `f_deffp` et `f_carfours` sont
portées. Lot 2 (distances et informations) : les 12 fiches `f_afdistab`,
`f_afdistat`, `f_afdistft`, `f_afdistfp`, `f_afdistp`, `f_afdistpv`,
`f_infveh`, `f_infptn`, `f_infbus`, `f_inftram`, `f_inftaxi` et `f_infvehlib`
sont portées. Lot 3 (statistiques et recherche) : les 5 fiches `f_stats`,
`f_statsv`, `f_statsat`, `f_statsc` et `f_rech` sont portées ; toutes les
fiches secondaires le sont désormais. Phase 7 (services système) : aide HTML
à côté de l'exécutable, F1 à la bonne rubrique ; phase 8 : règles
d'installation, installeur Windows, `.dmg` macOS, AppImage Linux (voir
« Installation et paquets »).
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
# macOS (paquet Centre-Ville.app, avec l'association des fichiers .cvw pour le Finder)
build/dev/app/Centre-Ville.app/Contents/MacOS/Centre-Ville maps/Démo.cvw
# Linux / Windows
build/dev/app/cvw maps/Démo.cvw
```

`cvw !` active le journal `cvw.log` (écrit à la fermeture), comme l'original.
Comme les tests du moteur, `cvw` remet à zéro la mémoire allouée par
`operator new` (`tests/support/allocation.cpp`) : le moteur d'origine laisse
des champs non initialisés (`centre_ville::FeuP` sans feux piétons...), et
poser le premier feu de piétons libérait sinon un pointeur indéterminé.
Les fichiers vont par défaut dans le dossier équivalent à `%LOCALAPPDATA%\Patquoi.fr\Centre-Ville\`
(`QStandardPaths::GenericDataLocation` + `Patquoi.fr/Centre-Ville` : le même
dossier sous Windows, `~/Library/Application Support/...` sous macOS,
`~/.local/share/...` sous Linux) ; `cvw.log` aussi (`stRepLocalAppData`, comme
l'original).

### Aide

La compilation copie `docs/guide` dans `html/` et `docs/tutorial` dans
`tutocv/`, à côté de l'exécutable comme l'installeur d'origine
(`{app}\bin\html`, `{app}\bin\tutocv`), et sous macOS dans
`Centre-Ville.app/Contents/Resources` (`stRepAide` dans `base.cpp` ;
`Contents/MacOS` ne doit contenir que du code, sinon la signature du paquet
échoue). La copie est refaite quand `docs/` change ; l'installation met la
même arborescence au même endroit. F1 (`HelpContext` de la fiche, table
`caCorresp` inchangée, 57 rubriques), Alt+Maj+F1 et « Sommaire » (index),
« Tutoriel » ouvrent la page dans le navigateur par défaut
(`QDesktopServices::openUrl`).

L'ancre `#...` ne survit pas à `openUrl` sur une adresse `file://` : le
système ouvre le fichier comme un document. Constaté sous macOS (Qt 6.11,
Edge par défaut : la page s'ouvre en haut) et sous Linux (Ubuntu 22.04 sans
bureau : `xdg-open`, en mode générique, retire `#...` des adresses
`file://` et ne passe que le chemin ; GNOME et KDE peuvent la garder) ;
sous Windows, d'après les sources de Qt 6.8, l'adresse et son ancre vont au
navigateur associé à `http` (`QWindowsServices::openUrl`), comme
l'original. Pour une solution
identique partout, `AffichePageHtml` écrit, pour une adresse locale avec
ancre, une petite page de redirection dans le dossier temporaire
(`cvw-aide-<ANCRE>.html`, `<meta http-equiv="refresh">` vers l'adresse
complète) et ouvre celle-ci : vérifié sous macOS avec Edge, Safari et Chrome
(le navigateur arrive à l'ancre), et de bout en bout avec le code de `cvw`
(F1 sur la fiche principale et dans Paramètres, Sommaire, Tutoriel) ; sous
Linux, avec l'AppImage sous Xvfb, F1 donne au navigateur la page de
redirection vers `usr/bin/html/index.html#PDLI` (le navigateur était factice :
le suivi de la redirection par un vrai navigateur sous Linux n'est pas
vérifié). Une visionneuse intégrée (`QTextBrowser`) aurait changé
l'interface.

## Installation et paquets

Le numéro de version n'est écrit qu'une fois : `project(VERSION 6.0.0)` du
`CMakeLists.txt` racine (`Info.plist`, ressources de version de `cvw.exe`,
lues par l'installeur Inno Setup, noms des paquets, et « À propos » :
`ImageVersion` « 6,0 », dessinée comme la bitmap d'origine « 5,4 » par
`ImageDeVersion`, et `LabelSousVersion` « ,0 »). Le format des fichiers
`.cvw` ne change pas (`VERSION 0x530`).

```bash
cmake --preset release -DCV_TESTS=OFF && cmake --build --preset release
cmake --install build/release --prefix "$PWD/build/installeur"   # préfixe absolu (script de déploiement de Qt)
```

| Système | Contenu installé | Paquet |
|---|---|---|
| Windows | `bin/cvw.exe` (icônes `Icon_0` et `Icon_1`, version : `cvw.rc.in`), DLL de Qt et de MSVC, `bin/qt.conf`, `plugins/`, `translations/` (windeployqt, par `qt_generate_deploy_app_script`), `bin/html`, `bin/tutocv` | `inno-setup/inscvw95.iss` (même `AppId`, association `.cvw`, cartes dans `{localappdata}\Patquoi.fr\Centre-Ville`) : `iscc /DStaging=<dossier installé> inno-setup\inscvw95.iss` |
| macOS | `Centre-Ville.app` (macdeployqt), aide et `qtbase_fr.qm` dans `Contents/Resources`, icônes `.icns` (`icones_macos.cmake`), type `.cvw` exporté (`Info.plist.in`), signature ad hoc | `cpack` dans le dossier de compilation : `Centre-Ville-<version>-macOS.dmg` (avec `Applications`, `Cartes/` et `licence.txt`) |
| Linux | `bin/cvw`, `bin/html`, `bin/tutocv`, `share/applications/fr.patquoi.CentreVille.desktop`, type MIME `application/x-centre-ville` (`*.cvw`), icônes (`linux/`, `ressources/icones`), cartes d'exemple (`share/centre-ville/cartes`) | AppImage : linuxdeploy et linuxdeploy-plugin-qt sur `DESTDIR=AppDir cmake --install`, plus le crochet `linux/apprun-hooks/centre-ville-cartes.sh` (`.github/workflows/build-installer.yml`) : au premier lancement, les cartes sont copiées, sans écraser un fichier existant, dans `~/.local/share/Patquoi.fr/Centre-Ville`, le dossier de `stRepLocalAppData` où Ouvrir et Enregistrer démarrent (comme l'installeur Windows dans `%LOCALAPPDATA%`) |

Les icônes viennent de `res/ico` (`tools/extrait_icones.py` en extrait les
images PNG de 256 × 256 : les PNG de `res/ico` ont une vignette parasite).
Sous Linux, l'aide est à côté de `cvw` comme ailleurs : pour une
installation hors AppImage, choisir un préfixe dédié (`/opt/centre-ville`).
`.github/workflows/build-installer.yml` compile sur les trois systèmes, sur
un tag `v*` ou à la main, et produit l'installeur Windows, le `.dmg`
(universel arm64 et x86_64, macOS 12 ou plus récent), l'AppImage x86_64
(construite sous Ubuntu 22.04) et les archives des sources ; il les joint à
une release seulement sur un tag. Ni signature ni notarisation.

## Organisation

| Fichier | Rôle |
|---|---|
| `main.cpp`, `application.*` | Port de `cvw.cpp` : fiches créées au démarrage dans le même ordre ; `frmSimulation` (Visible = True dans le `.dfm`) est affichée avant la création des autres, comme dans la VCL. Ouverture depuis le Finder (`QFileOpenEvent`). |
| `f_simul.*` | Port de la fiche principale. `VueSimulation` y implémente `IVueSimulation` (les accès du moteur à l'interface). |
| `f_dimzone.*`, `f_genres.*` | Portées. `Genere(rand()%NbX, rand()%NbY, ...)` est séquencé explicitement x, puis y, puis la direction, comme le binaire d'origine. |
| `f_propos`, `f_epidemie`, `f_defn`, `f_prm*` (5), `f_defv`, `f_defvp`, `f_deff`, `f_deffp`, `f_carfours` | Portées (lot 1). `ChargeFiche()` remplace le chargement du `.dfm` ; `ShellExecute` (adresse, courriel) -> `QDesktopServices::openUrl` ; `MessageBoxW` (Licence) -> `AfficheMessage` ; `((TComponent*)Sender)->Tag`/`Name`/`ClassName()` -> `TagDe`/`NomDe`/`NomClasseDe` ; `__except` -> `catch(...)`. `f_carfours` appelle `IncrementeNombre<feu>`, instanciée explicitement dans `engine/simul.cpp`. |
| `f_afdist*` (6), `f_inf*` (6) | Portées (lot 2) : fiches non modales au premier plan, ouvertes par les actions Distances/Détail (menus, barre d'outils, raccourcis), rafraîchies par `DrawGridSimulClick` et, pour les informations, par le moteur à chaque tour (`IVueSimulation::RafraichitFiche`). `Caption=<int>` -> `IntToStr` (`f_infptn`) ; `NumArretSvt=...` sorti de `ARRAYOFCONST` pour garder l'ordre d'évaluation de l'original (`f_infbus`, `f_inftram`). |
| `f_stats`, `f_statsv`, `f_statsat`, `f_statsc`, `f_rech` | Portées (lot 3). Évolution et Statistiques véhicules : non modales, au premier plan, rafraîchies par le moteur à chaque tour (`IVueSimulation::RafraichitFiche`) et fermées à l'arrêt. Composition et Fluidité et trafic : modales, créées à chaque ouverture puis libérées (`Release`) ; pendant Fluidité et trafic, `DrawGridSimulDrawCell` colore la carte par `CouleurCase` et la barre d'état détaille la case (`asMsgDetailCase`). Rechercher : non modale ; ses `CSpinEdit*Change` déplacent la case courante (`Col`/`Row`, donc `DrawGridSimulClick`), y compris à chaque tour pour l'objet suivi (bloc `frmRecherche` du moteur) : les fiches d'informations suivent ainsi l'objet. |
| `base.*` | Port de `src/base.*` : `AfficheMessage` (`QMessageBox`), `MessageDlg`, `stRepLocalAppData`, journal, aide (`stRepAide`, page de redirection pour l'ancre) ; `min`/`max`. |
| `grille/GrilleSimulation.*` | Remplace le `TDrawGrid` : voir l'en-tête pour le comportement reproduit. |
| `vcl/` | Sous-ensemble de la VCL au-dessus de Qt, pour garder le code des fiches tel quel. |
| `ui/` | GÉNÉRÉ par `tools/dfm2ui.py` : `.ui` des 33 fiches et `<fiche>_composants.h`. |
| `ressources/` | GÉNÉRÉ par `tools/extrait_images.py` : images des `.dfm` et index des `TVirtualImageList` ; `ressources/icones` : GÉNÉRÉ par `tools/extrait_icones.py`. |
| `Info.plist.in`, `icones_macos.cmake`, `cvw.rc.in`, `linux/` | Paquet macOS (type `.cvw`, icônes), ressources Windows (icônes, version), entrée de bureau et type MIME Linux. |

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
- Ajouts du lot 2 : `TStringGrid` (`grilletextes.cpp`, dessiné comme la VCL :
  cases fixes au style des en-têtes du système, traits `clSilver`, texte en
  (+2, +2), `Cells` creux comme le `TSparseList`, `ColWidths`/`RowHeights`,
  `FixedCols`/`FixedRows` (réduits à `RowCount-1` par `RowCount=`, comme
  `TCustomGrid::SetRowCount`), défilement par cases entières selon
  `ScrollBars`, case courante au clic, au clavier et à la molette ; police
  Symbol traduite en Unicode comme Windows le fait, d'où les flèches des
  tableaux de distances). `dfm2ui.py` en fait un `QAbstractScrollArea`.
  `TLabel::AutoSize` (`AdjustBounds` à chaque `Caption=` du code, bord droit
  fixe si aligné à droite ; la taille du `.dfm` est gardée au chargement).
  Fiches `bsSizeToolWin` : `Width`/`Height` comptent un cadre de 16 x 22
  pixels (celui que supposent les `.dfm` et le code des distances), retiré
  des `Constraints` de la fiche. `FormatFloat` : « 0 », « # », « . » et
  littéraux (« #.000% », « ###.## »...), arrondi de la moitié vers le haut,
  `NAN`/`INF`.
- Ajouts du lot 3 : `PopupMenu` de la fiche elle-même (`f_statsat`) : clic
  droit hors des contrôles qui ont leur propre menu, et raccourcis de ses
  éléments, `TWinControl::IsMenuKey` remontant jusqu'à la fiche (les flèches
  déplacent alors le curseur de la carte, même dans un champ numérique).
  `Color` de la fiche quand le `.dfm` la fixe (fond noir d'« Évolution » ;
  cela a aussi corrigé le fond sarcelle des priorités et le fond noir
  d'« À propos », restés à la couleur du thème aux lots 1 et 2) ; les couleurs
  système (`clBtnFace`) suivent toujours le thème. `ActiveControl` : focus
  initial de la fiche (`f_statsv` : « Vu » plutôt que la première grille ;
  aussi `f_defn`, `f_dimzone`, `f_prmbus`, `f_prmtram`, `f_prmtaxi`,
  `f_prmvehlib`). `TTabControl` : onglets d'un `QTabBar` tous visibles
  (texte élidé au besoin, sans boutons de défilement), cadre de la page
  dessiné sous les contrôles. `TPen::Mode`, `RGB()` de `windows.h`.
  `TPaintBox` (déjà présent) sert pour la première fois : `OnPaint` dans le
  `paintEvent`, sur un `TCanvas` persistant (`Pen` gardé d'un dessin à
  l'autre, comme le `Canvas` de la VCL).
- `TCSpinEdit` reproduit `CSPIN.pas` : le texte fait foi (base
  `QAbstractSpinBox` : sans type de valeur, Qt n'y touche jamais, alors que
  `QSpinBox` le bornait pendant la frappe et le remplaçait par sa valeur
  interne en quittant le champ). `Value` lit `StrToInt(Text)`, ou `MinValue`
  si le texte n'est pas un nombre ; `MinValue`/`MaxValue` sont de simples
  champs (ni le texte ni `OnChange` ne bougent) ; `CheckValue`, qui ne borne
  que si `MinValue≠MaxValue`, s'applique à `Value=`, aux flèches
  (`Value±Increment`, touches haut et bas comprises) et en quittant le champ
  vers un autre contrôle (`CMExit` ; pas quand la fenêtre perd l'activation,
  se ferme ou ouvre un menu) ; la frappe n'est pas bornée (chiffres, signes et
  séparateur décimal, comme `IsValidChar`) ; `OnChange` suit chaque
  changement du texte. Comme sous Windows, `Value=` (`WM_SETTEXT`) laisse le
  curseur au début du texte, sans sélection ; entrer dans le champ autrement
  qu'au clic sélectionne le texte (`CMEnter`, `AutoSelect`), mais pas quand la
  fenêtre retrouve l'activation (après une boîte de message) ; les flèches ne
  sont jamais grisées aux bornes (captures du lot 1 : seule différence, avec
  la sélection du champ actif après `Value=`). `dfm2ui.py` écrit les valeurs
  du `.dfm` en propriétés `vclMinValue`, `vclMaxValue`, `vclValue`
  (appliquée après les bornes, comme au chargement) et `vclIncrement`. Les
  contrôles fenêtrés (boutons, cases à
  cocher, boutons radio, curseurs) prennent le focus au clic comme sous
  Windows, y compris sous macOS, où Qt ne le leur donne qu'à la tabulation :
  sinon, cliquer sur OK ne ferait pas sortir du champ et la valeur tapée ne
  serait pas ramenée dans les bornes. Sous macOS, le contrôle cliqué garde
  donc l'anneau de focus, et Tab passe aussi par les boutons, cases et
  boutons radio, comme sous Windows, même sans l'accès clavier complet du
  système.
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
Les simulations des tests (lots 2 et 3, aide) sont jouées pas à pas, à graine
fixe : `Demarre` arrête le minuteur `TimerSimul` et le test joue chaque tour
par son gestionnaire (`JoueTour`), puis traite les événements comme la boucle
de l'application ; aucun tour ne peut ainsi tomber entre ce que le test
observe et ce qu'il vérifie (des vérifications échouaient de temps en temps
sous macOS en CI). Arrêt et Interruption, traités au tic suivant du minuteur,
le relancent. Seul le test Marche / Interruption / Arrêt garde le temps réel.
Un clic sur une case est suivi de son dessin, qui règle l'état des actions
(« Suivre... ») avant la touche suivante, comme pour l'utilisateur.
Lot 2, en simulation à graine fixe sur des copies de Géant (arrêts, files et
places de chaque sorte) et de Démo (parkings seulement) : les six fiches de distances,
ouvertes par menu, menu contextuel, raccourci ou action sur une case de
route (de voie de tram), affichent pour chaque ligne et direction la distance
du moteur (`DistParking`, `DistArretBus`...), couloirs de bus compris ; la
fiche prend la hauteur que lui donne `FormShow` (grille sans défilement
jusqu'à 10 lignes) ; un clic sur une autre case met le tableau à jour ;
toutes se ferment à l'arrêt. Les six fiches d'informations (non modales, au
premier plan), ouvertes sur un véhicule, un bus, un tram, un taxi, un vehlib
ou un piéton, montrent à chaque tour les valeurs du moteur pour la case
courante, l'utilisateur cliquant sur l'objet quand il change de case ; « Vu »
et Échap les ferment. À l'arrêt, la fiche ouverte garde son contenu (comme
l'original : rien ne la rafraîchit) ; le clic suivant sur la carte la vide.
Lot 3, en simulation à graine fixe sur des copies de Géant : Composition
(modale, barre d'outils puis Ctrl+Maj+R, « Vu » puis Échap) affiche la
composition calculée sur la carte (cases, routes, priorités, directions,
parkings, arrêts, voies de tram, places) ; Évolution (barre d'outils, puis
menu) est redessinée par le moteur à chaque tour, ses trois courbes et son
quadrillage sont aux positions que donnent `cv->Stats` (par nombre, par
pourcentage, quadrillage masqué, mode épidémique et sa légende) ;
Statistiques véhicules affiche les coordonnées des parkings puis, à chaque
tour, l'écart entre le plus long et le plus court des véhicules arrivés pour
chaque trajet, en cases puis en tours (clic sur l'onglet) ; les deux se
ferment à l'arrêt. Fluidité et trafic : refusée en simulation (message),
proposée par le bilan de l'interruption (« Oui ») puis ouverte par Ctrl+Maj+F :
fiche modale, couleur de chaque case de la carte (`CouleurCase` et pixel
dessiné) égale au calcul sur l'attente et le trafic du moteur, totaux et
moyennes, trajets (parkings de départ et d'arrivée), détail de la case dans
la barre d'état, boutons de déplacement (avec et sans les coins) et touches
du menu contextuel de la fiche (flèches, Origine ; Z sans effet) ; n° d'étape
tapé hors bornes : « Le n° d'étape est incorrect ! », cases noires et détail
vide quand une flèche déplace le curseur depuis le champ, puis numéro ramené
dans les bornes en quittant le champ (Tab). Rechercher
(barre d'outils, sur un parking) reprend les numéros de la case ; chaque sorte
d'élément fixe (parking, feu, arrêts, places et files) est retrouvée par son
bouton et son numéro (case courante, barre d'état de la fiche), ainsi que
des coordonnées ; le curseur clignote (dessiné une fois sur deux) ; « Suivre »
un véhicule, un piéton, un bus, un tram, un taxi et un vehlib (raccourcis du
menu contextuel de la carte) : à chaque tour, la case courante est sur
l'objet, la barre d'état de la fiche le dit, et la fiche d'informations
ouverte montre l'objet suivi, sans clic de l'utilisateur ; Échap ferme
Rechercher, « Suivre » la rouvre. Hors simulation, un numéro de parking
resté au-dessus du nombre de parkings n'est ni borné ni signalé par
`OnChange` quand `FormShow` règle `MaxValue` (la case courante ne bouge
pas) ; tapé hors bornes, il donne « n° incorrect » à chaque chiffre, puis
Tab le ramène au dernier parking, retrouvé.
Couche VCL (`controleCSpinEdit`, sur un `TCSpinEdit` seul) : `MinValue=` et
`MaxValue=` sans effet ni `OnChange` ; `Value=` borné, `OnChange` seulement si
le texte change ; frappe hors bornes acceptée (lettres refusées), ramenée
dans les bornes au clic sur un bouton mais pas à la perte d'activation de la
fenêtre ; texte vide (`Value` = `MinValue`) laissé vide ; flèches ; aucune
borne si `MinValue=MaxValue` ; `EditorEnabled=False` ; valeur du `.dfm`
bornée au chargement.
Aide (phase 7) : F1 sur la fiche principale, sur Rechercher (non modale) et
sur Paramètres (modale), bouton Aide, Alt+Maj+F1, menus Sommaire et Tutoriel
ouvrent la bonne page à la bonne rubrique (`QDesktopServices::openUrl`
intercepté par `setUrlHandler("file")`).
`tests/app/test_aide.cpp` (CTest `interface.aide`) : les 57 entrées de
`caCorresp`, lues dans `src/base.cpp` (l'original), ouvrent la page de
redirection vers `html/index.html#ANCRE` à côté de l'exécutable, et chaque
ancre existe dans `docs/guide/index.html` ; tous les `HelpContext` des `.ui`
ont leur rubrique ; un `HelpContext` inconnu n'ouvre rien ; `html/` et
`tutocv/` copiés à côté de `cvw` (dans le paquet sous macOS) sont identiques
à `docs/`.
`tests/app/test_cartes.cpp` (CTest `interface.cartes`) : le crochet des
cartes de l'AppImage, lancé comme par son `AppRun` (`sh`, `set -e`) sur une
AppDir et un `XDG_DATA_HOME` temporaires, copie les cartes de `maps/` sans
écraser une carte modifiée, une seule fois ; sous Linux, le dossier est celui
de `stRepLocalAppData` (Ouvrir et Enregistrer y démarrent : `interface.demo`).
Affichage natif de Windows (`policesEtStyleWindows`, sauté en « offscreen ») :
police Microsoft Sans Serif obtenue par les fiches, style « windowsvista » ;
le workflow « Recette Windows » lance le test dans cet affichage et joint les
captures de chaque fiche (en « offscreen », Windows n'a pas de polices).
« À propos » (`interface.demo`) : `ImageDeVersion(5, 4)` redessine la bitmap
d'origine au pixel près, et la fiche montre la version de `project(VERSION)`.
`CV_DOSSIER_CAPTURES=<dossier>` enregistre les captures de la fenêtre
principale (Démo au tour 0 et au tour 200 ; Géant en statistiques de fluidité,
`carte_fluidite_trafic.png`, et de trafic, `carte_trafic.png`) et des fiches
(`fiche_*.png`).

## Écarts connus avec l'original

Comportement :

- **Évolution en mode épidémique sans piéton** (`NbPietons` nul) : rien
  n'est tracé ; l'original divise par zéro (exception `EDivByZero` à chaque
  dessin).
- **« Zoom (-/+) » de Fluidité et trafic** (touche Z ou menu contextuel de la
  fiche) : sans effet ; l'original appelle `Click()` sur un `BitBtn22` qui
  n'existe plus depuis la v5.4 (violation d'accès, message d'erreur de la RTL).
- **Divisions par zéro** (fiches d'informations : pourcentages de parcours,
  de détour et d'attente quand le dénominateur est nul) : `NAN`/`INF`
  affichés par `FormatFloat`, au lieu de l'exception de la RTL de
  C++Builder (division par zéro et opération invalide démasquées).
- **Paramètres et fiches filles** : comme dans l'original, ouvrir une fiche
  fille (bus, tram, taxis, vehlibs) cache Paramètres puis la réaffiche, ce
  qui repasse par `FormShow` et recharge les valeurs du moteur (les saisies
  non appliquées sont perdues, comportement d'origine conservé). Seul écart :
  sous macOS, la console affiche alors « modalSession has been exited
  prematurely », sans effet.
- **Numéros tapés hors bornes** (le `TCSpinEdit` ne les borne qu'en quittant
  le champ, comme l'original) : l'original lisait alors hors d'un tableau ;
  le port s'en garde (`// portage`). Fluidité et trafic, n° d'étape hors de
  `0..NbParkings` : cases noires et détail de la case vide (l'original lisait
  hors de `Attente`/`Trafic` : violation d'accès ou couleurs quelconques,
  par exemple quand une flèche déplace le curseur depuis le champ ; le port
  plantait sans cette garde). Fiche Piéton pendant que Rechercher suit un
  numéro hors de `1..NbPietons` : aucun piéton suivi (piétons de la case).
  Paramètres des lignes de bus et de tram : un n° d'arrêt négatif est remis à
  0, comme un numéro trop grand.
- **Molette, Page précédente et Page suivante** dans un `TCSpinEdit` :
  `Value±Increment` (dix fois pour les pages), comme dans les champs
  numériques de Qt ; la VCL n'y réagissait pas.
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
- **Aide contextuelle** : le navigateur reçoit d'abord une page de
  redirection (`cvw-aide-<ANCRE>.html` du dossier temporaire), qui charge
  aussitôt la rubrique (voir « Aide »). Sous Windows, cette page s'ouvre avec
  l'application associée aux fichiers `.html` ; l'original lançait le
  navigateur associé à `http` (`HKCR\http\shell\open\command`).
- **Linux, aide de l'AppImage** (non vérifié) : un navigateur confiné (snap ou
  Flatpak, comme le Firefox d'Ubuntu) a son propre `/tmp` et ne lit ni l'aide
  de l'AppImage, montée sous `/tmp/.mount_*`, ni la page de redirection.
- **Distribution** : `cvw.32.exe` est abandonné (Qt 6 n'existe pas en
  32 bits) : Windows 10/11 x64, ou ARM64 avec l'émulation x64. Sous macOS,
  l'application est `Centre-Ville.app` (exécutable `Centre-Ville`).
  Traductions de Qt : `qt_fr.qm` dans le dossier de windeployqt, qui y fusionne
  `qtbase_fr.qm` (`application.cpp` essaie l'un puis l'autre).

Visuel :

- **Polices** : MS Sans Serif et Small Fonts sont des polices « raster » que
  Qt ne dessine pas, même sous Windows (où il les trouve, puis prend une
  police de secours plus large). `FamillesQt` (`vcl/graphiques.cpp`) met
  devant leurs équivalents TrueType : Microsoft Sans Serif, aux mêmes mesures
  (fournie par Windows et par macOS), et Verdana ou Tahoma pour Small Fonts
  (barre d'état, numéros d'arrêts sur la carte) ; sous Linux, police du
  système (substitutions de `application.cpp`). Les tailles en pixels de la
  VCL sont conservées.
- **Style sous Windows** : « windowsvista » (contrôles de Windows 10) plutôt
  que « windows11 », le style par défaut de Qt sous Windows 11, dont les
  flèches côte à côte cachaient la valeur des champs numériques étroits.
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
  les couleurs fixées dans les `.dfm` (fond noir d'« À propos » et
  d'« Évolution », fond sarcelle des priorités, couleurs des lignes de bus et
  de tram...).
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
  Paramètres, des lignes de bus et de tram, des fiches d'informations, de la
  première colonne de Composition, de Fluidité et trafic, des numéros à
  quatre chiffres de Rechercher...) ;
  les valeurs écrites par le code (`AutoSize`) restent entières, quitte à
  recouvrir la fin du libellé voisin.
- **Tableaux de distances** (`TStringGrid`) : cases fixes au style des
  en-têtes du système (Windows : en-têtes du thème), flèches de la police
  Symbol dessinées dans la police du système ; nombres alignés par les
  espaces de « %5d », comme l'original, mais avec d'autres largeurs. Cadre
  et barre de titre des fenêtres outils (`bsSizeToolWin`) du système :
  `Height=26+21*RowCount` donne une grille de `RowCount` lignes entières,
  comme avec le cadre que suppose le code d'origine (`src/A_FAIRE.txt`
  prévoyait de revoir ce calcul).
- **Onglets** (`TTabControl` de Statistiques véhicules) : style du système ;
  avec la police de remplacement, les deux libellés sont élidés (« … ») pour
  tenir dans la largeur de la VCL.
- **Évolution** : traits de `QPainter` ; comme GDI, `LineTo` n'allume pas le
  point d'arrivée, mais les pixels des segments obliques peuvent différer
  d'un pixel de ceux de Windows.

## Comportements d'origine conservés (lot 2)

Règle d'or : ces bogues de l'original sont portés tels quels ; ils sont repris
dans `CHANGELOG.md` (version 6.0.0), à corriger après la bascule.

- **Fiches d'informations à l'arrêt** : rien ne les rafraîchit ; elles
  montrent le dernier objet jusqu'au prochain clic sur la carte rechargée.
- **Tableau de distances vide** (carte sans arrêt, file ou place de la
  sorte) : `RowCount=1` ramène `FixedRows` à 0 (`TCustomGrid::SetRowCount`),
  et la grille garde ensuite sa ligne d'en-tête défilante, même sur une autre
  carte.
- **Files de parkings** (`f_afdistfp`) : la hauteur (26 et 283) ne compte pas
  la case « Avec couloirs de bus » (`f_afdistp` utilise 43 et 300) : la
  grille est plus courte de 17 pixels et défile.
- **Bus et tram** : « Prochain arrêt ... depuis le tour » lit
  `TourDepartArretSvt` du premier bus (du premier tram) ; « Tours effectifs »
  du tram affiche `TourCrt`.
- **Piéton** : dans un tram, un taxi ou un vehlib, le cadre prend les
  coordonnées dans `cv->Bus` ; la situation « vers une file de vehlibs » est
  testée sur `FileTaxiTrv` ; position « ans un vehlib ».
- **Moteur : blocage des vehlibs** (trouvé en testant les fiches sur Géant ;
  `engine/` non modifié) : `vehlib::ReservePlaceVehlibCible` tire au hasard
  une place libre sans limite d'essais. Sur Géant (29 vehlibs pour 32
  places), quand plus de vehlibs reçoivent un passager qu'il ne reste de
  places libres, la boucle ne se termine jamais et l'application se fige :
  selon la graine (`cvsim maps/Géant.cvw --graine 123` : avant le tour 50 ;
  123, 135 et 136 sur les graines 100 à 139 en 300 tours). Le test
  d'interface fixe donc la graine (1) pour ses simulations.
- **Taxi** : la distance à la tête de file est lue en
  `DistFileTaxi[direction][file]` (indices inversés) : valeur fausse, et
  lecture hors des tableaux quand la case a plus de directions que la carte
  n'a de files de taxis.

## Comportements d'origine conservés (lot 3)

- **Rechercher un piéton** monté dans un taxi : la case courante va au taxi,
  mais le texte donne les coordonnées de `cv->Bus[NumTaxi-1]` (lecture hors
  du tableau des bus si `NumTaxi > NbBus`) ; dans un vehlib, case courante
  et texte viennent de `cv->Bus[NumVehlib-1]` ; dans un tram, il est « non
  utilisé » (le commentaire v5.4.1 du code cite le tram, mais ne le traite
  pas). Comme la fiche Piéton lit `CSpinEditPieton`, elle montre le piéton
  suivi même quand la case en contient un autre.
- **Places de taxi, de parking et de vehlib** non définies : la barre d'état
  de Rechercher garde le texte précédent ; « Taxi » et « Vehlib » ne testent
  pas `Defini()`.
- **Statistiques véhicules** : la grille n'est remplie qu'au tour qui suit
  l'ouverture (`FormShow` n'écrit que les en-têtes) ; un trajet n'ayant
  qu'un véhicule arrivé, ou des véhicules au même parcours, affiche 0.
- **Évolution** : redessinée entre deux tours (fenêtre redimensionnée ou
  découverte), son dernier segment va vers `Stats[TourCrt%NbToursStats]`,
  les statistiques du tour à venir, pas encore calculées (celles d'il y a
  `NbToursStats` tours). Par nombre de véhicules, le quadrillage marque tous
  les 10 véhicules : sur Géant (plus de 2 000 véhicules), ses points forment
  des traits verticaux continus.
- **Fluidité et trafic** : les flèches, Origine, Fin, Page précédente et Page
  suivante (raccourcis du menu contextuel de la fiche) déplacent le curseur de
  la carte depuis n'importe quel contrôle, champs numériques compris ; Échap
  ne ferme pas la fiche (aucun bouton Cancel). Un n° d'étape tapé hors de
  `0..NbParkings` affiche « Le n° d'étape est incorrect ! » (à chaque chiffre
  qui le laisse hors bornes) ; il n'est ramené dans `1..NbParkings` qu'en
  quittant le champ (d'ici là, `0` donne les statistiques de tous les
  parkings).

## Comportements d'origine conservés (`TCSpinEdit`)

- **Entrée** (bouton par défaut) valide la valeur tapée telle quelle : la
  VCL n'envoie pas `CM_EXIT`, le champ n'est donc pas borné (le clic sur OK,
  lui, fait sortir du champ). La vérification d'intégrité de Marche signale
  une partie des valeurs incorrectes (négatives, notamment).
- **Texte vide, « - » ou « + »** : `Value` vaut `MinValue`, et le texte reste
  tel quel en quittant le champ (`CheckValue(MinValue)` ne change rien).
- **`MinValue` = `MaxValue`** : aucune borne, ni à l'affectation ni en
  quittant le champ (Dimensions de la zone ; parkings de Fluidité et trafic
  sur une carte à un seul parking).
- **Rechercher** : `FormShow` règle les `MaxValue` d'après la carte sans
  toucher aux numéros : un numéro resté au-dessus (carte précédente plus
  grande) reste affiché, jusqu'à la flèche ou la sortie du champ.
- **Bus et tram** : un n° d'arrêt trop grand, nul ou vide est remis à 0 dès la
  frappe (`OnChange`), un arrêt non défini remplacé par le suivant.
