//---------------------------------------------------------------------------
// f_simul_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_simul.dfm : ne pas modifier
//
// COMPOSANTS_frmSimulation(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmSimulation(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_simul_composantsH
#define f_simul_composantsH

#define COMPOSANTS_frmSimulation(X) \
  X(TStatusBar, StatusBar) \
  X(TDrawGrid, DrawGridSimul) \
  X(TPanel, PanelHaut) \
  X(TToolBar, ToolBarFichier) \
  X(TToolButton, ToolButtonNouveau) \
  X(TToolButton, ToolButtonOuvrir) \
  X(TToolButton, ToolButtonEnregistrer) \
  X(TToolBar, ToolBarSimulation) \
  X(TToolButton, ToolButtonParametres) \
  X(TToolButton, ToolButtonVerifierReseau) \
  X(TToolButton, ToolButtonMarche) \
  X(TToolButton, ToolButtonPause) \
  X(TToolButton, ToolButtonArret) \
  X(TToolButton, ToolButtonSanglante) \
  X(TToolButton, ToolButtonEpidemie) \
  X(TToolBar, ToolBarAffichage) \
  X(TToolButton, ToolButtonRafraichir) \
  X(TToolButton, ToolButtonAffDir) \
  X(TToolButton, ToolButtonAffSign) \
  X(TToolButton, ToolButtonAffRoutes) \
  X(TToolButton, ToolButtonAffVeh) \
  X(TToolButton, ToolButtonAffPtn) \
  X(TToolButton, ToolButtonAffQuadr) \
  X(TToolButton, ToolButtonAffEnv) \
  X(TToolButton, ToolButtonAffInfct) \
  X(TToolBar, ToolBarInformations) \
  X(TToolButton, ToolButtonRechercher) \
  X(TToolButton, ToolButtonDetail) \
  X(TToolButton, ToolButtonSuivi) \
  X(TToolButton, ToolButtonDistances) \
  X(TToolBar, ToolBarEditionCase) \
  X(TToolButton, ToolButtonNord) \
  X(TToolButton, ToolButtonEst) \
  X(TToolButton, ToolButtonSud) \
  X(TToolButton, ToolButtonOuest) \
  X(TToolButton, ToolButtonEffacerRoute) \
  X(TToolButton, ToolButtonTramVersNord) \
  X(TToolButton, ToolButtonTramVersEst) \
  X(TToolButton, ToolButtonTramVersSud) \
  X(TToolButton, ToolButtonTramVersOuest) \
  X(TToolButton, ToolButtonEffacerVoieTram) \
  X(TToolButton, ToolButtonToutDroit) \
  X(TToolButton, ToolButtonPriorites) \
  X(TToolButton, ToolButtonParking) \
  X(TToolButton, ToolButtonDepotBus) \
  X(TToolButton, ToolButtonPassagePietons) \
  X(TToolButton, ToolButtonArretBusTram) \
  X(TToolButton, ToolButtonPassageBus) \
  X(TToolButton, ToolButtonPlaceTaxi) \
  X(TToolButton, ToolButtonPlacePark) \
  X(TToolButton, ToolButtonPlaceVehlib) \
  X(TToolButton, ToolButtonDeplacerFeu) \
  X(TToolButton, ToolButtonNatureDecors) \
  X(TToolBar, ToolBarStatistiques) \
  X(TToolButton, ToolButtonComposition) \
  X(TToolButton, ToolButtonAffStatsEtapes) \
  X(TToolButton, ToolButtonEvolution) \
  X(TToolButton, ToolButtonFluiditeTrafic) \
  X(TToolBar, ToolBarEditionSelection) \
  X(TToolButton, ToolButtonSegmentRoute) \
  X(TToolButton, ToolButtonSegmentTram) \
  X(TToolButton, ToolButtonCoins) \
  X(TToolButton, ToolButtonPeripherique) \
  X(TToolButton, ToolButtonCarrefours) \
  X(TToolButton, ToolButtonEffacerRoutesSelection) \
  X(TToolButton, ToolButtonEffacerVoiesTramSelection) \
  X(TToolButton, ToolButtonDefinirNature) \
  X(TTimer, TimerSimul) \
  X(TTimer, TimerClignotementErreur) \
  X(TOpenDialog, OpenDialog) \
  X(TSaveDialog, SaveDialog) \
  X(TTimer, TimerClignotementLocalisation) \
  X(TImageCollection, ImageCollection) \
  X(TVirtualImageList, VirtualImageList16x16) \
  X(TVirtualImageList, VirtualImageList8x8) \
  X(TVirtualImageList, VirtualImageListGlyphes) \
  X(TActionList, ActionList) \
  X(TAction, ActionNouveau) \
  X(TAction, ActionOuvrir) \
  X(TAction, ActionEnregistrer) \
  X(TAction, ActionEnregistrerSous) \
  X(TAction, ActionVerifierApresOuverture) \
  X(TAction, ActionQuitter) \
  X(TAction, ActionAfficherDirections) \
  X(TAction, ActionAfficherSignalisation) \
  X(TAction, ActionAfficherVoies) \
  X(TAction, ActionAfficherVehicules) \
  X(TAction, ActionAfficherPietons) \
  X(TAction, ActionAfficherQuadrillage) \
  X(TAction, ActionAfficherEnvironnement) \
  X(TAction, ActionParametres) \
  X(TAction, ActionVerifierReseau) \
  X(TAction, ActionMarche) \
  X(TAction, ActionInterruption) \
  X(TAction, ActionArret) \
  X(TAction, ActionVerifierAvantSimulation) \
  X(TAction, ActionEvolution) \
  X(TAction, ActionStatsAffQuadrillage) \
  X(TAction, ActionStatsAffParVehicule) \
  X(TAction, ActionStatsAffParPourcent) \
  X(TAction, ActionStatsAff10Tours) \
  X(TAction, ActionStatsAff20Tours) \
  X(TAction, ActionStatsAff50Tours) \
  X(TAction, ActionStatsAff100Tours) \
  X(TAction, ActionStatsAff200Tours) \
  X(TAction, ActionStatsAff500Tours) \
  X(TAction, ActionFluiditeTrafic) \
  X(TAction, ActionComposition) \
  X(TAction, ActionStatsParkings) \
  X(TAction, ActionAPropos) \
  X(TAction, ActionSommaire) \
  X(TAction, ActionDirectionHaut) \
  X(TAction, ActionDirectionDroite) \
  X(TAction, ActionDirectionBas) \
  X(TAction, ActionDirectionGauche) \
  X(TAction, ActionEffacerRoute) \
  X(TAction, ActionDirectionHautTram) \
  X(TAction, ActionDirectionDroiteTram) \
  X(TAction, ActionDirectionBasTram) \
  X(TAction, ActionDirectionGaucheTram) \
  X(TAction, ActionEffacerVoieTram) \
  X(TAction, ActionToutDroit) \
  X(TAction, ActionPriorites) \
  X(TAction, ActionPrioritesPietons) \
  X(TAction, ActionParking) \
  X(TAction, ActionDepotBus) \
  X(TAction, ActionPassagePietons) \
  X(TAction, ActionArretBusTram) \
  X(TAction, ActionPassageBus) \
  X(TAction, ActionPlaceTaxi) \
  X(TAction, ActionPlacePark) \
  X(TAction, ActionPlaceVehlib) \
  X(TAction, ActionDeplacerFeuSource) \
  X(TAction, ActionDeplacerFeuDestination) \
  X(TAction, ActionSegmentRoute) \
  X(TAction, ActionSegmentTram) \
  X(TAction, ActionCoinSuperieurDroit) \
  X(TAction, ActionCoinInferieurGauche) \
  X(TAction, ActionCoinSuperieurGauche) \
  X(TAction, ActionCoinInferieurDroit) \
  X(TAction, ActionPeripheriqueExterieur) \
  X(TAction, ActionPeripheriqueInterieur) \
  X(TAction, ActionCarrefours) \
  X(TAction, ActionEffacerRoutes) \
  X(TAction, ActionEffacerVoiesTram) \
  X(TAction, ActionDefinirNature) \
  X(TAction, ActionRechercher) \
  X(TAction, ActionDetailVehicule) \
  X(TAction, ActionDetailBus) \
  X(TAction, ActionDetailTram) \
  X(TAction, ActionDetailPieton) \
  X(TAction, ActionDetailTaxi) \
  X(TAction, ActionDetailVehlib) \
  X(TAction, ActionSuivreVehicule) \
  X(TAction, ActionSuivreBus) \
  X(TAction, ActionSuivreTram) \
  X(TAction, ActionSuivreTaxi) \
  X(TAction, ActionSuivreVehlib) \
  X(TAction, ActionSuivrePieton1) \
  X(TAction, ActionSuivrePieton2) \
  X(TAction, ActionDistancesParkings) \
  X(TAction, ActionDistancesArretsBus) \
  X(TAction, ActionDistancesArretsTram) \
  X(TAction, ActionDistancesFilesTaxi) \
  X(TAction, ActionDistancesFilesPark) \
  X(TAction, ActionDistancesPlacesVehlib) \
  X(TAction, ActionNatureTrottoir) \
  X(TAction, ActionDecorsArbre) \
  X(TAction, ActionDecorsPelouse) \
  X(TAction, ActionNatureImmeuble) \
  X(TAction, ActionCopierVers) \
  X(TAction, ActionSanglante) \
  X(TAction, ActionResurrection) \
  X(TAction, ActionTutoriel) \
  X(TAction, ActionRafraichir) \
  X(TAction, ActionEpidemie) \
  X(TAction, ActionAfficherInfection) \
  X(TMainMenu, MainMenu) \
  X(TMenuItem, MenuFichier) \
  X(TMenuItem, MenuItemNouveau) \
  X(TMenuItem, Sep6) \
  X(TMenuItem, MenuItemOuvrir) \
  X(TMenuItem, MenuItemVerifierIntegriteApresOuverture) \
  X(TMenuItem, Sep7) \
  X(TMenuItem, MenuItemEnregistrer) \
  X(TMenuItem, MenuItemEnregistrerSous) \
  X(TMenuItem, Sep8) \
  X(TMenuItem, MenuItemQuitter) \
  X(TMenuItem, MenuAffichage) \
  X(TMenuItem, MenuItemRafraichir) \
  X(TMenuItem, SepO) \
  X(TMenuItem, MenuItemAffichageDirections) \
  X(TMenuItem, MenuItemAffichageSignalisation) \
  X(TMenuItem, MenuItemAffichageRoutes) \
  X(TMenuItem, MenuItemAffichageVehicules) \
  X(TMenuItem, MenuItemAffichagePietons) \
  X(TMenuItem, MenuItemAffichageQuadrillage) \
  X(TMenuItem, MenuItemAffichageEnvironnement) \
  X(TMenuItem, MenuSimulation) \
  X(TMenuItem, MenuItemParametres) \
  X(TMenuItem, MenuItemVerifierReseau) \
  X(TMenuItem, Sep3) \
  X(TMenuItem, MenuItemMarche) \
  X(TMenuItem, MenuItemPause) \
  X(TMenuItem, MenuItemArret) \
  X(TMenuItem, MenuItemVerifierIntegriteAvantSimulation) \
  X(TMenuItem, Sep4) \
  X(TMenuItem, MenuItemSanglant) \
  X(TMenuItem, Epidemie1) \
  X(TMenuItem, MenuItemResurrection) \
  X(TMenuItem, MenuStatistiques) \
  X(TMenuItem, MenuItemEvolution) \
  X(TMenuItem, MenuAffichageStatistiques) \
  X(TMenuItem, MenuItemQuadrillageStatistiques) \
  X(TMenuItem, SepK) \
  X(TMenuItem, MenuItemParNbVehicules) \
  X(TMenuItem, MenuItemParPourcentage) \
  X(TMenuItem, SepL) \
  X(TMenuItem, MenuItem10DrnTours) \
  X(TMenuItem, MenuItem20DrnTours) \
  X(TMenuItem, MenuItem50DrnTours) \
  X(TMenuItem, MenuItem100DrnTours) \
  X(TMenuItem, MenuItem200DrnTours) \
  X(TMenuItem, MenuItem500DrnTours) \
  X(TMenuItem, SepD) \
  X(TMenuItem, MenuItemFluiditeTrafic) \
  X(TMenuItem, MenuItemComposition) \
  X(TMenuItem, MenuItemStatistiquesEtapes) \
  X(TMenuItem, MenuInformations) \
  X(TMenuItem, MenuItemAPropos) \
  X(TMenuItem, MenuItemAide) \
  X(TMenuItem, MenuItemTutoriel) \
  X(TPopupMenu, PopupMenuSimulation) \
  X(TMenuItem, MenuItemRechercher) \
  X(TMenuItem, MenuDetail) \
  X(TMenuItem, MenuItemInfosVehicule) \
  X(TMenuItem, MenuItemInfosBus) \
  X(TMenuItem, MenuItemInfosTram) \
  X(TMenuItem, MenuItemInfosPieton) \
  X(TMenuItem, MenuItemInfosTaxi) \
  X(TMenuItem, MenuItemInfosVehlib) \
  X(TMenuItem, MenuSuivi) \
  X(TMenuItem, MenuItemSuivreCeVehicule) \
  X(TMenuItem, MenuItemSuivreCeBus) \
  X(TMenuItem, MenuItemSuivreCeTram) \
  X(TMenuItem, MenuItemSuivreCeTaxi) \
  X(TMenuItem, MenuItemSuivreCeVehlib) \
  X(TMenuItem, SepB) \
  X(TMenuItem, MenuItemSuivreLe1erPieton) \
  X(TMenuItem, MenuItemSuivreLe2emePieton) \
  X(TMenuItem, MenuDistances) \
  X(TMenuItem, MenuItemInfosDistancesEtapes) \
  X(TMenuItem, MenuItemInfosDistancesArretsBus) \
  X(TMenuItem, MenuItemInfosDistancesArretsTram) \
  X(TMenuItem, MenuItemInfosDistancesFilesTaxi) \
  X(TMenuItem, MenuItemInfosDistancesFilesPark) \
  X(TMenuItem, MenuItemInfosDistancesFilesVehlib) \
  X(TPopupMenu, PopupMenuBarreOutils) \
  X(TMenuItem, MenuItemFichier) \
  X(TMenuItem, MenuItemAffichage) \
  X(TMenuItem, MenuItemSimulation) \
  X(TMenuItem, MenuItemStatistiques) \
  X(TMenuItem, MenuItemInformations) \
  X(TMenuItem, SepN) \
  X(TMenuItem, MenuItemEditionCase) \
  X(TMenuItem, MenuItemEditionSelection) \
  X(TPopupMenu, PopupMenuEditionCase) \
  X(TMenuItem, MenuItemDirectionHaut) \
  X(TMenuItem, MenuItemDirectionDroite) \
  X(TMenuItem, MenuItemDirectionBas) \
  X(TMenuItem, MenuItemDirectionGauche) \
  X(TMenuItem, MenuItemEffacerRoute) \
  X(TMenuItem, SepA) \
  X(TMenuItem, MenuItemProlongerHaut) \
  X(TMenuItem, MenuItemProlongerDroite) \
  X(TMenuItem, MenuItemProlongerBas) \
  X(TMenuItem, MenuItemProlongerGauche) \
  X(TMenuItem, MenuItemEffacerVoieTram) \
  X(TMenuItem, SepF) \
  X(TMenuItem, MenuItemToutDroit) \
  X(TMenuItem, MenuItemPriorites) \
  X(TMenuItem, MenuItemPrioritesPietons) \
  X(TMenuItem, MenuItemEtape) \
  X(TMenuItem, MenuItemDepotBus) \
  X(TMenuItem, MenuItemPassagePietons) \
  X(TMenuItem, MenuItemArretBusTram) \
  X(TMenuItem, MenuItemPassageBus) \
  X(TMenuItem, MenuItemPlaceTaxi) \
  X(TMenuItem, MenuItemPlacePark) \
  X(TMenuItem, MenuItemPlaceVehlib) \
  X(TMenuItem, MenuItemDeplacerFeu) \
  X(TMenuItem, SepP) \
  X(TMenuItem, MenuNatureDecors) \
  X(TMenuItem, MenuItemTrottoir) \
  X(TMenuItem, SepQ) \
  X(TMenuItem, MenuItemArbre) \
  X(TMenuItem, MenuItemPelouse) \
  X(TMenuItem, SepR) \
  X(TMenuItem, MenuItemImmeuble) \
  X(TPopupMenu, PopupMenuEditionSelection) \
  X(TMenuItem, MenuItemSegmentRoute) \
  X(TMenuItem, MenuItemSegmentVoieTram) \
  X(TMenuItem, MenuRoutesCoins) \
  X(TMenuItem, MenuItemRouteBordSuperieurDroit) \
  X(TMenuItem, MenuItemRouteBordInferieurGauche) \
  X(TMenuItem, MenuItemRouteBordSuperieurGauche) \
  X(TMenuItem, MenuItemRouteBordInferieurDroit) \
  X(TMenuItem, MenuPeripherique) \
  X(TMenuItem, PriphriqueExtrieur1) \
  X(TMenuItem, PriphriqueIntrieur1) \
  X(TMenuItem, SepI) \
  X(TMenuItem, MenuItemCarrefours) \
  X(TMenuItem, SepJ) \
  X(TMenuItem, MenuItemEffacerRoutes) \
  X(TMenuItem, MenuItemEffacerVoiesTram) \
  X(TMenuItem, SepM) \
  X(TMenuItem, MenuItemDefinirNature) \
  X(TMenuItem, SepS) \
  X(TMenuItem, MenuItemCopierVers) \
  X(TPopupMenu, PopupMenuDistances) \
  X(TMenuItem, MenuItemDistancesEtapes) \
  X(TMenuItem, MenuItemDistancesArretsBus) \
  X(TMenuItem, MenuItemDistancesArretsTram) \
  X(TMenuItem, MenuItemDistancesFilesTaxi) \
  X(TMenuItem, MenuItemDistancesFilesPark) \
  X(TMenuItem, MenuItemDistancesPlacesVehlib) \
  X(TPopupMenu, PopupMenuDetail) \
  X(TMenuItem, MenuItemDetailVehicule) \
  X(TMenuItem, MenuItemDetailBus) \
  X(TMenuItem, MenuItemDetailTram) \
  X(TMenuItem, MenuItemDetailPieton) \
  X(TMenuItem, MenuItemDetailTaxi) \
  X(TMenuItem, MenuItemDetailVehlib) \
  X(TPopupMenu, PopupMenuSuivi) \
  X(TMenuItem, MenuItemSuiviVehicule) \
  X(TMenuItem, MenuItemSuiviBus) \
  X(TMenuItem, MenuItemSuiviTram) \
  X(TMenuItem, MenuItemSuiviTaxi) \
  X(TMenuItem, MenuItemSuiviVehlib) \
  X(TMenuItem, SepZ) \
  X(TMenuItem, MenuItemSuiviPieton1) \
  X(TMenuItem, MenuItemSuiviPieton2) \
  X(TPopupMenu, PopupMenuCoins) \
  X(TMenuItem, MenuItemAngleSuperieurDroit) \
  X(TMenuItem, MenuItemAngleInferieurGauche) \
  X(TMenuItem, MenuItemAngleSuperieurGauche) \
  X(TMenuItem, MenuItemAngleInferieurDroit) \
  X(TPopupMenu, PopupMenuPeripherique) \
  X(TMenuItem, MenuItemExterieur) \
  X(TMenuItem, MenuItemInterieur) \
  X(TPopupMenu, PopupMenuDecors) \
  X(TMenuItem, MenuItemDecorsTrottoir) \
  X(TMenuItem, SepX) \
  X(TMenuItem, MenuItemDecorsArbre) \
  X(TMenuItem, MenuItemDecorsPelouse) \
  X(TMenuItem, SepY) \
  X(TMenuItem, MenuItemDecorsImmeuble) \

#define EVENEMENTS_frmSimulation(X) \
  X(this, OnCloseQuery, FormCloseQuery) \
  X(this, OnCreate, FormCreate) \
  X(this, OnDestroy, FormDestroy) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnKeyUp, FormKeyUp) \
  X(this, OnShow, FormShow) \
  X(StatusBar, OnResize, StatusBarResize) \
  X(DrawGridSimul, OnClick, DrawGridSimulClick) \
  X(DrawGridSimul, OnDblClick, DrawGridSimulDblClick) \
  X(DrawGridSimul, OnDrawCell, DrawGridSimulDrawCell) \
  X(TimerSimul, OnTimer, TimerSimulTimer) \
  X(TimerClignotementErreur, OnTimer, TimerClignotementErreurTimer) \
  X(TimerClignotementLocalisation, OnTimer, TimerClignotementLocalisationTimer) \
  X(ActionNouveau, OnExecute, ActionNouveauExecute) \
  X(ActionOuvrir, OnExecute, ActionOuvrirExecute) \
  X(ActionEnregistrer, OnExecute, ActionEnregistrerExecute) \
  X(ActionEnregistrerSous, OnExecute, ActionEnregistrerSousExecute) \
  X(ActionVerifierApresOuverture, OnExecute, ActionVerifierApresOuvertureExecute) \
  X(ActionQuitter, OnExecute, ActionQuitterExecute) \
  X(ActionAfficherDirections, OnExecute, ActionAfficherDirectionsExecute) \
  X(ActionAfficherSignalisation, OnExecute, ActionAfficherSignalisationExecute) \
  X(ActionAfficherVoies, OnExecute, ActionAfficherVoiesExecute) \
  X(ActionAfficherVehicules, OnExecute, ActionAfficherVehiculesExecute) \
  X(ActionAfficherPietons, OnExecute, ActionAfficherPietonsExecute) \
  X(ActionAfficherQuadrillage, OnExecute, ActionAfficherQuadrillageExecute) \
  X(ActionAfficherEnvironnement, OnExecute, ActionAfficherEnvironnementExecute) \
  X(ActionParametres, OnExecute, ActionParametresExecute) \
  X(ActionVerifierReseau, OnExecute, ActionVerifierReseauExecute) \
  X(ActionMarche, OnExecute, ActionMarcheExecute) \
  X(ActionInterruption, OnExecute, ActionInterruptionExecute) \
  X(ActionArret, OnExecute, ActionArretExecute) \
  X(ActionVerifierAvantSimulation, OnExecute, ActionVerifierAvantSimulationExecute) \
  X(ActionEvolution, OnExecute, ActionEvolutionExecute) \
  X(ActionStatsAffQuadrillage, OnExecute, ActionStatsAffQuadrillageExecute) \
  X(ActionStatsAffParVehicule, OnExecute, ActionStatsAffParVehiculeExecute) \
  X(ActionStatsAffParPourcent, OnExecute, ActionStatsAffParPourcentExecute) \
  X(ActionStatsAff10Tours, OnExecute, ActionStatsAff10ToursExecute) \
  X(ActionStatsAff20Tours, OnExecute, ActionStatsAff20ToursExecute) \
  X(ActionStatsAff50Tours, OnExecute, ActionStatsAff50ToursExecute) \
  X(ActionStatsAff100Tours, OnExecute, ActionStatsAff100ToursExecute) \
  X(ActionStatsAff200Tours, OnExecute, ActionStatsAff200ToursExecute) \
  X(ActionStatsAff500Tours, OnExecute, ActionStatsAff500ToursExecute) \
  X(ActionFluiditeTrafic, OnExecute, ActionFluiditeTraficExecute) \
  X(ActionComposition, OnExecute, ActionCompositionExecute) \
  X(ActionStatsParkings, OnExecute, ActionStatsParkingsExecute) \
  X(ActionAPropos, OnExecute, ActionAProposExecute) \
  X(ActionSommaire, OnExecute, ActionSommaireExecute) \
  X(ActionDirectionHaut, OnExecute, ActionDirectionHautExecute) \
  X(ActionDirectionDroite, OnExecute, ActionDirectionDroiteExecute) \
  X(ActionDirectionBas, OnExecute, ActionDirectionBasExecute) \
  X(ActionDirectionGauche, OnExecute, ActionDirectionGaucheExecute) \
  X(ActionEffacerRoute, OnExecute, ActionEffacerRouteExecute) \
  X(ActionDirectionHautTram, OnExecute, ActionDirectionHautTramExecute) \
  X(ActionDirectionDroiteTram, OnExecute, ActionDirectionDroiteTramExecute) \
  X(ActionDirectionBasTram, OnExecute, ActionDirectionBasTramExecute) \
  X(ActionDirectionGaucheTram, OnExecute, ActionDirectionGaucheTramExecute) \
  X(ActionEffacerVoieTram, OnExecute, ActionEffacerVoieTramExecute) \
  X(ActionToutDroit, OnExecute, ActionToutDroitExecute) \
  X(ActionPriorites, OnExecute, ActionPrioritesExecute) \
  X(ActionPrioritesPietons, OnExecute, ActionPrioritesPietonsExecute) \
  X(ActionParking, OnExecute, ActionParkingExecute) \
  X(ActionDepotBus, OnExecute, ActionDepotBusExecute) \
  X(ActionPassagePietons, OnExecute, ActionPassagePietonsExecute) \
  X(ActionArretBusTram, OnExecute, ActionArretBusTramExecute) \
  X(ActionPassageBus, OnExecute, ActionPassageBusExecute) \
  X(ActionPlaceTaxi, OnExecute, ActionPlaceTaxiExecute) \
  X(ActionPlacePark, OnExecute, ActionPlaceParkExecute) \
  X(ActionPlaceVehlib, OnExecute, ActionPlaceVehlibExecute) \
  X(ActionDeplacerFeuSource, OnExecute, ActionDeplacerFeuSourceExecute) \
  X(ActionDeplacerFeuDestination, OnExecute, ActionDeplacerFeuDestinationExecute) \
  X(ActionSegmentRoute, OnExecute, ActionSegmentRouteExecute) \
  X(ActionSegmentTram, OnExecute, ActionSegmentTramExecute) \
  X(ActionCoinSuperieurDroit, OnExecute, ActionCoinSuperieurDroitExecute) \
  X(ActionCoinInferieurGauche, OnExecute, ActionCoinInferieurGaucheExecute) \
  X(ActionCoinSuperieurGauche, OnExecute, ActionCoinSuperieurGaucheExecute) \
  X(ActionCoinInferieurDroit, OnExecute, ActionCoinInferieurDroitExecute) \
  X(ActionPeripheriqueExterieur, OnExecute, ActionPeripheriqueExterieurExecute) \
  X(ActionPeripheriqueInterieur, OnExecute, ActionPeripheriqueInterieurExecute) \
  X(ActionCarrefours, OnExecute, ActionCarrefoursExecute) \
  X(ActionEffacerRoutes, OnExecute, ActionEffacerRoutesExecute) \
  X(ActionEffacerVoiesTram, OnExecute, ActionEffacerVoiesTramExecute) \
  X(ActionDefinirNature, OnExecute, ActionDefinirNatureExecute) \
  X(ActionRechercher, OnExecute, ActionRechercherExecute) \
  X(ActionDetailVehicule, OnExecute, ActionDetailVehiculeExecute) \
  X(ActionDetailBus, OnExecute, ActionDetailBusExecute) \
  X(ActionDetailTram, OnExecute, ActionDetailTramExecute) \
  X(ActionDetailPieton, OnExecute, ActionDetailPietonExecute) \
  X(ActionDetailTaxi, OnExecute, ActionDetailTaxiExecute) \
  X(ActionDetailVehlib, OnExecute, ActionDetailVehlibExecute) \
  X(ActionSuivreVehicule, OnExecute, ActionSuivreVehiculeExecute) \
  X(ActionSuivreBus, OnExecute, ActionSuivreBusExecute) \
  X(ActionSuivreTram, OnExecute, ActionSuivreTramExecute) \
  X(ActionSuivreTaxi, OnExecute, ActionSuivreTaxiExecute) \
  X(ActionSuivreVehlib, OnExecute, ActionSuivreVehlibExecute) \
  X(ActionSuivrePieton1, OnExecute, ActionSuivrePieton1Execute) \
  X(ActionSuivrePieton2, OnExecute, ActionSuivrePieton2Execute) \
  X(ActionDistancesParkings, OnExecute, ActionDistancesParkingsExecute) \
  X(ActionDistancesArretsBus, OnExecute, ActionDistancesArretsBusExecute) \
  X(ActionDistancesArretsTram, OnExecute, ActionDistancesArretsTramExecute) \
  X(ActionDistancesFilesTaxi, OnExecute, ActionDistancesFilesTaxiExecute) \
  X(ActionDistancesFilesPark, OnExecute, ActionDistancesFilesParkExecute) \
  X(ActionDistancesPlacesVehlib, OnExecute, ActionDistancesPlacesVehlibExecute) \
  X(ActionNatureTrottoir, OnExecute, ActionNatureTrottoirExecute) \
  X(ActionDecorsArbre, OnExecute, ActionDecorsArbreExecute) \
  X(ActionDecorsPelouse, OnExecute, ActionDecorsPelouseExecute) \
  X(ActionNatureImmeuble, OnExecute, ActionNatureImmeubleExecute) \
  X(ActionCopierVers, OnExecute, ActionCopierVersExecute) \
  X(ActionSanglante, OnExecute, ActionSanglanteExecute) \
  X(ActionResurrection, OnExecute, ActionResurrectionExecute) \
  X(ActionTutoriel, OnExecute, ActionTutorielExecute) \
  X(ActionRafraichir, OnExecute, ActionRafraichirExecute) \
  X(ActionEpidemie, OnExecute, ActionEpidemieExecute) \
  X(ActionAfficherInfection, OnExecute, ActionAfficherInfectionExecute) \
  X(PopupMenuBarreOutils, OnPopup, PopupMenuBarreOutilsPopup) \
  X(MenuItemFichier, OnClick, MenuItemBarreOutilsClick) \
  X(MenuItemAffichage, OnClick, MenuItemBarreOutilsClick) \
  X(MenuItemSimulation, OnClick, MenuItemBarreOutilsClick) \
  X(MenuItemStatistiques, OnClick, MenuItemBarreOutilsClick) \
  X(MenuItemInformations, OnClick, MenuItemBarreOutilsClick) \
  X(MenuItemEditionCase, OnClick, MenuItemBarreOutilsClick) \
  X(MenuItemEditionSelection, OnClick, MenuItemBarreOutilsClick) \

#endif
