//---------------------------------------------------------------------------
#ifndef f_simulH
#define f_simulH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL (ActnList, ComCtrls, Menus...)
#include "grille/GrilleSimulation.h" // portage : TDrawGrid
#include "vue.h" // portage : IVueSimulation
//#include <Controls.hpp> // portage
//#include <Dialogs.hpp> // portage
//#include <ExtCtrls.hpp> // portage
//#include <Grids.hpp> // portage
//#include <ImgList.hpp> // portage
//#include <Menus.hpp> // portage
//#include <ToolWin.hpp> // portage
//#include <Registry.hpp> // portage
//#include <System.Actions.hpp> // portage
//#include <System.ImageList.hpp> // portage
//#include <Vcl.BaseImageCollection.hpp> // portage
//#include <Vcl.ImageCollection.hpp> // portage
//#include <Vcl.VirtualImageList.hpp> // portage
//---------------------------------------------------------------------------
#include "simul.h"
//---------------------------------------------------------------------------
// v5.4 : les glyphes de TSpeedButtons sont chargées manuellement (HDPI)
//---------------------------------------------------------------------------
#define GLYPHE_DEF    0
#define GLYPHE_DEFF   1
#define GLYPHE_DEFFP  2
#define GLYPHE_SELF   3
#define GLYPHE_DEFB   4
#define GLYPHE_DEFTR  5
#define GLYPHE_DEFTX  6
#define GLYPHE_DEFVL  7
#define GLYPHE_DIR    8
//----------------------------------------------------------------------------------------------------------------------
// Types énumérés. /!\ v5.4 : le type char a été forcé pour que l'énuméré ne prenne pas la taille d'un entier (4 octets)
//----------------------------------------------------------------------------------------------------------------------
// pour RafraichitBoutonsEdition(...) v3.5
enum affichage : unsigned char {aff_aucun, aff_dir, aff_sign, aff_veh=4, aff_ptn=8, aff_voie=16, aff_quadr=32, aff_env=64, aff_infct=128}; // v3.8.1 (aff_env). v5.4.1 (unsigned + aff_infct)
enum type_demande : char {tdRien, tdPause, tdArret};
enum operation_avant : char {oaNouveau, oaOuvrir, oaQuitter};
enum mode_edition : char {meAucun, meCase, meSelection}; // pour RafraichitBoutonEdition. v3.5
//---------------------------------------------------------------------------
// Utilisée dans la classe TfrmSimulation
extern const AnsiString asTitreCV;
//---------------------------------------------------------------------------
// Classe de la fiche
class TfrmSimulation : public TFormPrincipale // portage : TForm -> QMainWindow
{
public: // portage : __published (composants liés par COMPOSANTS_frmSimulation)
  TTimer *TimerSimul;
  TTimer *TimerClignotementLocalisation;
  TTimer *TimerClignotementErreur;
  TStatusBar *StatusBar;
  TSaveDialog *SaveDialog;
  TOpenDialog *OpenDialog;
  TDrawGrid *DrawGridSimul;
  TMainMenu *MainMenu;
  TPopupMenu *PopupMenuEditionCase;
  TPopupMenu *PopupMenuSimulation;
  TPopupMenu *PopupMenuEditionSelection;
  TPopupMenu *PopupMenuBarreOutils;
  TMenuItem *MenuFichier;
  TMenuItem *MenuItemNouveau;
  TMenuItem *MenuItemOuvrir;
  TMenuItem *MenuItemEnregistrer;
  TMenuItem *MenuItemEnregistrerSous;
  TMenuItem *MenuItemQuitter;
  TMenuItem *MenuSimulation;
  TMenuItem *MenuItemParametres;
  TMenuItem *MenuInformations;
  TMenuItem *MenuItemAPropos;
  TMenuItem *MenuItemAide;
  TMenuItem *MenuAffichageStatistiques;
  TMenuItem *MenuItemQuadrillageStatistiques;
  TMenuItem *MenuItem100DrnTours;
  TMenuItem *SepL;
  TMenuItem *MenuItem200DrnTours;
  TMenuItem *MenuItem50DrnTours;
  TMenuItem *MenuItem20DrnTours;
  TMenuItem *MenuItem10DrnTours;
  TMenuItem *MenuItem500DrnTours;
  TMenuItem *SepK;
  TMenuItem *MenuItemParNbVehicules;
  TMenuItem *MenuItemParPourcentage;
  TMenuItem *MenuStatistiques;
  TMenuItem *MenuItemEvolution;
  TMenuItem *MenuItemStatistiquesEtapes;
  TMenuItem *MenuAffichage;
  TMenuItem *MenuItemAffichageDirections;
  TMenuItem *MenuItemAffichageSignalisation;
  TMenuItem *MenuItemAffichageRoutes;
  TMenuItem *MenuItemAffichageVehicules;
  TMenuItem *MenuItemAffichageQuadrillage;
  TMenuItem *Sep3;
  TMenuItem *MenuItemMarche;
  TMenuItem *MenuItemPause;
  TMenuItem *MenuItemArret;
  TMenuItem *MenuItemVerifierReseau;
  TMenuItem *Sep6;
  TMenuItem *Sep7;
  TMenuItem *Sep8;
  TMenuItem *MenuItemVerifierIntegriteApresOuverture;
  TMenuItem *MenuItemVerifierIntegriteAvantSimulation;
  TMenuItem *MenuItemDirectionHaut;
  TMenuItem *MenuItemDirectionDroite;
  TMenuItem *MenuItemDirectionBas;
  TMenuItem *MenuItemDirectionGauche;
  TMenuItem *MenuItemToutDroit;
  TMenuItem *MenuItemEtape;
  TMenuItem *MenuItemPriorites;
  TMenuItem *MenuItemInfosVehicule;
  TMenuItem *MenuItemInfosDistancesEtapes;
  TMenuItem *MenuItemSuivreCeVehicule;
  TMenuItem *MenuItemComposition;
  TMenuItem *MenuItemPassagePietons;
  TMenuItem *MenuItemDeplacerFeu;
  TMenuItem *MenuItemAffichagePietons;
  TMenuItem *SepD;
  TMenuItem *MenuItemFluiditeTrafic;
  TMenuItem *MenuItemArretBusTram;
  TMenuItem *MenuItemInfosDistancesArretsBus;
  TMenuItem *SepB;
  TMenuItem *MenuItemDepotBus;
  TMenuItem *MenuItemInfosBus;
  TMenuItem *MenuItemSuivreCeBus;
  TMenuItem *MenuItemInfosPieton;
  TMenuItem *MenuItemSuivreLe1erPieton;
  TMenuItem *MenuItemSuivreLe2emePieton;
  TMenuItem *MenuItemPassageBus;
  TMenuItem *MenuItemSegmentRoute;
  TMenuItem *MenuPeripherique;
  TMenuItem *SepI;
  TMenuItem *MenuItemCarrefours;
  TMenuItem *SepJ;
  TMenuItem *MenuItemEffacerRoutes;
  TMenuItem *SepA;
  TMenuItem *SepF;
  TMenuItem *MenuItemProlongerHaut;
  TMenuItem *MenuItemProlongerDroite;
  TMenuItem *MenuItemProlongerBas;
  TMenuItem *MenuItemProlongerGauche;
  TMenuItem *MenuItemEffacerVoieTram;
  TMenuItem *MenuItemAffichage;
  TMenuItem *MenuItemStatistiques;
  TMenuItem *MenuItemEditionCase;
  TMenuItem *MenuItemSimulation;
  TMenuItem *MenuItemEditionSelection;
  TMenuItem *SepN;
  TMenuItem *SepO;
  TMenuItem *MenuItemInformations;
  TMenuItem *MenuItemInfosTram;
  TMenuItem *MenuItemSuivreCeTram;
  TMenuItem *MenuItemInfosDistancesArretsTram;
  TMenuItem *MenuItemFichier;
  TMenuItem *MenuDetail;
  TMenuItem *MenuSuivi;
  TMenuItem *MenuDistances;
  TMenuItem *MenuItemRechercher;
  TMenuItem *MenuItemSegmentVoieTram;
  TMenuItem *MenuRoutesCoins;
  TMenuItem *MenuItemRouteBordSuperieurDroit;
  TMenuItem *MenuItemRouteBordInferieurGauche;
  TMenuItem *MenuItemRouteBordSuperieurGauche;
  TMenuItem *MenuItemRouteBordInferieurDroit;
  TMenuItem *MenuItemEffacerVoiesTram;
  TMenuItem *MenuItemEffacerRoute;
  TMenuItem *MenuItemPlaceTaxi;
  TMenuItem *MenuItemPlacePark;
  TMenuItem *MenuItemPlaceVehlib;
  TMenuItem *MenuItemInfosDistancesFilesTaxi;
  TMenuItem *MenuItemInfosTaxi;
  TMenuItem *MenuItemSuivreCeTaxi;
  TMenuItem *MenuItemInfosVehlib;
  TMenuItem *MenuItemSuivreCeVehlib;
  TMenuItem *MenuItemDistancesArretsBus;
  TMenuItem *MenuItemDistancesArretsTram;
  TMenuItem *MenuItemDistancesFilesPark;
  TMenuItem *MenuItemDistancesEtapes;
  TMenuItem *MenuItemDetailVehicule;
  TMenuItem *MenuItemDetailBus;
  TMenuItem *MenuItemDetailTram;
  TMenuItem *MenuItemDetailPieton;
  TMenuItem *MenuItemDetailTaxi;
  TMenuItem *MenuItemDetailVehlib;
  TMenuItem *MenuItemSuiviVehicule;
  TMenuItem *MenuItemSuiviBus;
  TMenuItem *MenuItemSuiviTram;
  TMenuItem *MenuItemSuiviTaxi;
  TMenuItem *MenuItemSuiviVehlib;
  TMenuItem *SepZ;
  TMenuItem *MenuItemSuiviPieton1;
  TMenuItem *MenuItemSuiviPieton2;
  TMenuItem *MenuItemAngleSuperieurDroit;
  TMenuItem *MenuItemAngleInferieurGauche;
  TMenuItem *MenuItemAngleSuperieurGauche;
  TMenuItem *MenuItemAngleInferieurDroit;
  TMenuItem *SepM;
  TMenuItem *MenuItemDefinirNature;
  TMenuItem *MenuItemAffichageEnvironnement;
  TMenuItem *SepP;
  TMenuItem *MenuNatureDecors;
  TMenuItem *MenuItemArbre;
  TMenuItem *MenuItemPelouse;
  TMenuItem *SepR;
  TMenuItem *MenuItemTrottoir;
  TMenuItem *SepQ;
  TMenuItem *MenuItemImmeuble;
  TMenuItem *PriphriqueExtrieur1;
  TMenuItem *PriphriqueIntrieur1;
  TMenuItem *MenuItemExterieur;
  TMenuItem *MenuItemInterieur;
  TMenuItem *MenuItemDecorsTrottoir;
  TMenuItem *SepX;
  TMenuItem *MenuItemDecorsArbre;
  TMenuItem *MenuItemDecorsPelouse;
  TMenuItem *SepY;
  TMenuItem *MenuItemDecorsImmeuble;
  TMenuItem *SepS;
  TMenuItem *MenuItemCopierVers;
  TMenuItem *Sep4;
  TMenuItem *MenuItemSanglant;
  TMenuItem *MenuItemResurrection;
  TMenuItem *MenuItemDistancesFilesTaxi;
  TMenuItem *MenuItemInfosDistancesFilesPark;
  TMenuItem *MenuItemTutoriel;
  TMenuItem *MenuItemRafraichir;
  TMenuItem *MenuItemPrioritesPietons;
  TMenuItem *MenuItemInfosDistancesFilesVehlib;
        TMenuItem *MenuItemDistancesPlacesVehlib;
  TToolBar *ToolBarFichier;
  TToolBar *ToolBarAffichage;
  TToolBar *ToolBarInformations;
  TToolBar *ToolBarSimulation;
  TToolBar *ToolBarEditionCase;
  TToolBar *ToolBarStatistiques;
  TToolBar *ToolBarEditionSelection;
  TToolButton *ToolButtonNouveau;
  TToolButton *ToolButtonOuvrir;
  TToolButton *ToolButtonEnregistrer;
  TToolButton *ToolButtonAffDir;
  TToolButton *ToolButtonAffSign;
  TToolButton *ToolButtonAffRoutes;
  TToolButton *ToolButtonAffVeh;
  TToolButton *ToolButtonAffPtn;
  TToolButton *ToolButtonAffQuadr;
  TToolButton *ToolButtonRechercher;
  TToolButton *ToolButtonDetail;
  TToolButton *ToolButtonSuivi;
  TToolButton *ToolButtonDistances;
  TToolButton *ToolButtonParametres;
  TToolButton *ToolButtonVerifierReseau;
  TToolButton *ToolButtonMarche;
  TToolButton *ToolButtonPause;
  TToolButton *ToolButtonArret;
  TToolButton *ToolButtonNord;
  TToolButton *ToolButtonEst;
  TToolButton *ToolButtonSud;
  TToolButton *ToolButtonOuest;
  TToolButton *ToolButtonEffacerRoute;
  TToolButton *ToolButtonTramVersNord;
  TToolButton *ToolButtonTramVersEst;
  TToolButton *ToolButtonTramVersSud;
  TToolButton *ToolButtonTramVersOuest;
  TToolButton *ToolButtonEffacerVoieTram;
  TToolButton *ToolButtonToutDroit;
  TToolButton *ToolButtonPriorites;
  TToolButton *ToolButtonParking;
  TToolButton *ToolButtonDepotBus;
  TToolButton *ToolButtonPassagePietons;
  TToolButton *ToolButtonArretBusTram;
  TToolButton *ToolButtonPassageBus;
  TToolButton *ToolButtonPlaceTaxi;
  TToolButton *ToolButtonPlacePark;
  TToolButton *ToolButtonDeplacerFeu;
  TToolButton *ToolButtonComposition;
  TToolButton *ToolButtonAffStatsEtapes;
  TToolButton *ToolButtonEvolution;
  TToolButton *ToolButtonFluiditeTrafic;
  TToolButton *ToolButtonSegmentRoute;
  TToolButton *ToolButtonSegmentTram;
  TToolButton *ToolButtonCoins;
  TToolButton *ToolButtonPeripherique;
  TToolButton *ToolButtonCarrefours;
  TToolButton *ToolButtonEffacerRoutesSelection;
  TToolButton *ToolButtonEffacerVoiesTramSelection;
  TToolButton *ToolButtonDefinirNature;
  TToolButton *ToolButtonAffEnv;
  TToolButton *ToolButtonSanglante;
  TToolButton *ToolButtonNatureDecors;
  TToolButton *ToolButtonRafraichir;
  TToolButton *ToolButtonPlaceVehlib;
  TPopupMenu *PopupMenuDistances;
  TPopupMenu *PopupMenuDetail;
  TPopupMenu *PopupMenuSuivi;
  TPopupMenu *PopupMenuCoins;
  TPopupMenu *PopupMenuPeripherique;
  TPopupMenu *PopupMenuDecors;
  TActionList *ActionList;
  TAction *ActionNouveau;
  TAction *ActionOuvrir;
  TAction *ActionEnregistrer;
  TAction *ActionEnregistrerSous;
  TAction *ActionVerifierApresOuverture;
  TAction *ActionQuitter;
  TAction *ActionAfficherDirections;
  TAction *ActionAfficherSignalisation;
  TAction *ActionAfficherVoies;
  TAction *ActionAfficherVehicules;
  TAction *ActionAfficherPietons;
  TAction *ActionAfficherQuadrillage;
  TAction *ActionAfficherEnvironnement;
  TAction *ActionParametres;
  TAction *ActionVerifierReseau;
  TAction *ActionMarche;
  TAction *ActionInterruption;
  TAction *ActionArret;
  TAction *ActionVerifierAvantSimulation;
  TAction *ActionEvolution;
  TAction *ActionStatsAffQuadrillage;
  TAction *ActionStatsAffParVehicule;
  TAction *ActionStatsAffParPourcent;
  TAction *ActionStatsAff10Tours;
  TAction *ActionStatsAff20Tours;
  TAction *ActionStatsAff50Tours;
  TAction *ActionStatsAff100Tours;
  TAction *ActionStatsAff200Tours;
  TAction *ActionStatsAff500Tours;
  TAction *ActionFluiditeTrafic;
  TAction *ActionComposition;
  TAction *ActionStatsParkings;
  TAction *ActionAPropos;
  TAction *ActionSommaire;
  TAction *ActionDirectionHaut;
  TAction *ActionDirectionDroite;
  TAction *ActionDirectionBas;
  TAction *ActionDirectionGauche;
  TAction *ActionEffacerRoute;
  TAction *ActionDirectionHautTram;
  TAction *ActionDirectionBasTram;
  TAction *ActionDirectionDroiteTram;
  TAction *ActionDirectionGaucheTram;
  TAction *ActionEffacerVoieTram;
  TAction *ActionToutDroit;
  TAction *ActionPriorites;
  TAction *ActionParking;
  TAction *ActionDepotBus;
  TAction *ActionPassagePietons;
  TAction *ActionArretBusTram;
  TAction *ActionPassageBus;
  TAction *ActionPlaceTaxi;
  TAction *ActionPlacePark;
  TAction *ActionDeplacerFeuSource;
  TAction *ActionDeplacerFeuDestination;
  TAction *ActionSegmentRoute;
  TAction *ActionSegmentTram;
  TAction *ActionCoinSuperieurDroit;
  TAction *ActionCoinInferieurDroit;
  TAction *ActionCoinSuperieurGauche;
  TAction *ActionCoinInferieurGauche;
  TAction *ActionCarrefours;
  TAction *ActionEffacerRoutes;
  TAction *ActionEffacerVoiesTram;
  TAction *ActionDefinirNature;
  TAction *ActionPeripheriqueExterieur;
  TAction *ActionPeripheriqueInterieur;
  TAction *ActionRechercher;
  TAction *ActionDetailVehicule;
  TAction *ActionDetailBus;
  TAction *ActionDetailTram;
  TAction *ActionDetailPieton;
  TAction *ActionDetailTaxi;
  TAction *ActionDetailVehlib;
  TAction *ActionSuivreVehicule;
  TAction *ActionSuivreBus;
  TAction *ActionSuivreTram;
  TAction *ActionSuivreTaxi;
  TAction *ActionSuivreVehlib;
  TAction *ActionSuivrePieton1;
  TAction *ActionSuivrePieton2;
  TAction *ActionDistancesParkings;
  TAction *ActionDistancesArretsBus;
  TAction *ActionDistancesArretsTram;
  TAction *ActionDistancesFilesTaxi;
  TAction *ActionDistancesFilesPark;
  TAction *ActionDistancesPlacesVehlib;
  TAction *ActionDecorsArbre;
  TAction *ActionDecorsPelouse;
  TAction *ActionNatureTrottoir;
  TAction *ActionNatureImmeuble;
  TAction *ActionCopierVers;
  TAction *ActionSanglante;
  TAction *ActionResurrection;
  TAction *ActionTutoriel;
  TAction *ActionRafraichir;
  TAction *ActionPrioritesPietons;
  TAction *ActionPlaceVehlib;
  TImageCollection *ImageCollection; // v5.4
  TVirtualImageList *VirtualImageList16x16;
	TVirtualImageList *VirtualImageList8x8;
	TVirtualImageList *VirtualImageListGlyphes;
	TPanel *PanelHaut;
	TAction *ActionEpidemie;
	TMenuItem *Epidemie1;
	TToolButton *ToolButtonEpidemie;
	TAction *ActionAfficherInfection;
	TToolButton *ToolButtonAffInfct; // v5.4
  void DrawGridSimulDrawCell(TObject *Sender, int Col,
		int Row, TRect &Rect, TGridDrawState State);
  void FormCreate(TObject *Sender);
  void FormDestroy(TObject *Sender);
  void DrawGridSimulDblClick(TObject *Sender);
  void FormShow(TObject *Sender);
  void DrawGridSimulClick(TObject *Sender);
  void TimerSimulTimer(TObject *Sender);
  void FormCloseQuery(TObject *Sender, bool &CanClose);
  void MenuItemBarreOutilsClick(TObject *Sender);
  void PopupMenuBarreOutilsPopup(TObject *Sender);
  void FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
  void TimerClignotementErreurTimer(TObject *Sender);
  void ActionQuitterExecute(TObject *Sender);
  void ActionVerifierApresOuvertureExecute(TObject *Sender);
  void ActionNouveauExecute(TObject *Sender);
  void ActionOuvrirExecute(TObject *Sender);
  void ActionEnregistrerExecute(TObject *Sender);
  void ActionEnregistrerSousExecute(TObject *Sender);
  void ActionAfficherDirectionsExecute(TObject *Sender);
  void ActionAfficherSignalisationExecute(TObject *Sender);
  void ActionAfficherVoiesExecute(TObject *Sender);
  void ActionAfficherVehiculesExecute(TObject *Sender);
  void ActionAfficherPietonsExecute(TObject *Sender);
  void ActionAfficherQuadrillageExecute(TObject *Sender);
  void ActionAfficherEnvironnementExecute(TObject *Sender);
  void ActionParametresExecute(TObject *Sender);
  void ActionVerifierReseauExecute(TObject *Sender);
  void ActionMarcheExecute(TObject *Sender);
  void ActionInterruptionExecute(TObject *Sender);
  void ActionArretExecute(TObject *Sender);
  void ActionVerifierAvantSimulationExecute(TObject *Sender);
  void ActionEvolutionExecute(TObject *Sender);
  void ActionStatsAff10ToursExecute(TObject *Sender);
  void ActionStatsAff20ToursExecute(TObject *Sender);
  void ActionStatsAff50ToursExecute(TObject *Sender);
  void ActionStatsAff100ToursExecute(TObject *Sender);
  void ActionStatsAff200ToursExecute(TObject *Sender);
  void ActionStatsAff500ToursExecute(TObject *Sender);
  void ActionStatsAffParVehiculeExecute(TObject *Sender);
  void ActionStatsAffParPourcentExecute(TObject *Sender);
  void ActionStatsAffQuadrillageExecute(TObject *Sender);
  void ActionFluiditeTraficExecute(TObject *Sender);
  void ActionCompositionExecute(TObject *Sender);
  void ActionStatsParkingsExecute(TObject *Sender);
  void ActionAProposExecute(TObject *Sender);
  void ActionSommaireExecute(TObject *Sender);
  void ActionDirectionHautExecute(TObject *Sender);
  void ActionDirectionDroiteExecute(TObject *Sender);
  void ActionDirectionBasExecute(TObject *Sender);
  void ActionDirectionGaucheExecute(TObject *Sender);
  void ActionEffacerRouteExecute(TObject *Sender);
  void ActionDirectionHautTramExecute(TObject *Sender);
  void ActionDirectionDroiteTramExecute(TObject *Sender);
  void ActionDirectionBasTramExecute(TObject *Sender);
  void ActionDirectionGaucheTramExecute(TObject *Sender);
  void ActionEffacerVoieTramExecute(TObject *Sender);
  void ActionToutDroitExecute(TObject *Sender);
  void ActionPrioritesExecute(TObject *Sender);
  void ActionParkingExecute(TObject *Sender);
  void ActionDepotBusExecute(TObject *Sender);
  void ActionPassagePietonsExecute(TObject *Sender);
  void ActionArretBusTramExecute(TObject *Sender);
  void ActionPassageBusExecute(TObject *Sender);
  void ActionPlaceTaxiExecute(TObject *Sender);
  void ActionPlaceParkExecute(TObject *Sender);
  void ActionDeplacerFeuSourceExecute(TObject *Sender);
  void ActionDeplacerFeuDestinationExecute(TObject *Sender);
  void ActionSegmentRouteExecute(TObject *Sender);
  void ActionSegmentTramExecute(TObject *Sender);
  void ActionCoinSuperieurDroitExecute(TObject *Sender);
  void ActionCoinInferieurGaucheExecute(TObject *Sender);
  void ActionCoinSuperieurGaucheExecute(TObject *Sender);
  void ActionCoinInferieurDroitExecute(TObject *Sender);
  void ActionPeripheriqueExterieurExecute(TObject *Sender);
  void ActionPeripheriqueInterieurExecute(TObject *Sender);
  void ActionCarrefoursExecute(TObject *Sender);
  void ActionEffacerRoutesExecute(TObject *Sender);
  void ActionEffacerVoiesTramExecute(TObject *Sender);
  void ActionDefinirNatureExecute(TObject *Sender);
  void ActionRechercherExecute(TObject *Sender);
  void ActionDetailVehiculeExecute(TObject *Sender);
  void ActionDetailBusExecute(TObject *Sender);
  void ActionDetailTramExecute(TObject *Sender);
  void ActionDetailPietonExecute(TObject *Sender);
  void ActionDetailTaxiExecute(TObject *Sender);
  void ActionDetailVehlibExecute(TObject *Sender);
  void ActionDistancesParkingsExecute(TObject *Sender);
  void ActionDistancesArretsBusExecute(TObject *Sender);
  void ActionDistancesArretsTramExecute(TObject *Sender);
  void ActionDistancesFilesTaxiExecute(TObject *Sender);
  void ActionSuivreVehiculeExecute(TObject *Sender);
  void ActionSuivreBusExecute(TObject *Sender);
  void ActionSuivreTramExecute(TObject *Sender);
  void ActionSuivreTaxiExecute(TObject *Sender);
  void ActionSuivreVehlibExecute(TObject *Sender);
  void ActionSuivrePieton1Execute(TObject *Sender);
  void ActionSuivrePieton2Execute(TObject *Sender);
  void ActionDecorsArbreExecute(TObject *Sender);
  void ActionDecorsPelouseExecute(TObject *Sender);
  void ActionNatureTrottoirExecute(TObject *Sender);
  void ActionNatureImmeubleExecute(TObject *Sender);
  void ActionCopierVersExecute(TObject *Sender);
  void ActionSanglanteExecute(TObject *Sender);
  void ActionResurrectionExecute(TObject *Sender);
  void TimerClignotementLocalisationTimer(TObject *Sender);
  void ActionDistancesFilesParkExecute(TObject *Sender);
  void ActionTutorielExecute(TObject *Sender);
  void ActionRafraichirExecute(TObject *Sender);
  void StatusBarResize(TObject *Sender);
  void ActionPrioritesPietonsExecute(TObject *Sender);
  void ActionPlaceVehlibExecute(TObject *Sender);
  void ActionDistancesPlacesVehlibExecute(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
	void ActionEpidemieExecute(TObject *Sender);
	void ActionAfficherInfectionExecute(TObject *Sender);
private:
  affichage Affichage;
  bool SimulEnCours,
       SimulModifiee,
       EnregistrerAvant,
       DeplacementFeuEnCours,
       RedimensionnementEnCours;
  int xErr, yErr, xDeplFeu, yDeplFeu,
      xCrt, yCrt,
      xPrc, yPrc; // Détecteurs de direction du curseur. v3.5
  direction DirOrigVoieTram; // v3.5
  // v5.4 : conversion BCB6>BCBX : (AnsiString>String)
  String asNomFichAuto;
  AnsiString asMsgErr;
  void RedimensionneZone();
  bool EnregistreModificationsAvant(operation_avant Operation);
  bool VerifieIntegrite();
  bool VerifieReseau();
  void AfficheMsgInfoNonDisp(const AnsiString asTitre); // v3.7
  // Définition de la propriété SimulEnCours
  bool SimulEstEnCours() { return SimulEnCours; };
  void MarcheArretSimul(bool EnCours)
   {
    SimulEnCours=EnCours;
    if (EnCours)
     cv->TopChrono();
    else
     cv->StopChrono(); // v3.9.2
    if (!EnCours) Application->Title=asTitreCV;

    if ((!Pause)&&(!EnCours))
     {
      DrawGridSimul->Options << goRangeSelect;
      DrawGridSimul->PopupMenu=PopupMenuEditionCase;
     }
    else
     {
      DrawGridSimul->Options >> goRangeSelect;
      DrawGridSimul->PopupMenu=PopupMenuSimulation;
     }

    ToolButtonDetail->Enabled=Pause||EnCours;
    ToolButtonSuivi->Enabled=Pause||EnCours;
    ToolButtonDistances->Enabled=Pause||EnCours;
    ToolButtonNatureDecors->Enabled=(!Pause)&&(!EnCours); // v5.1.1 : Oubli

    MenuDetail->Enabled=Pause||EnCours;
    MenuSuivi->Enabled=Pause||EnCours;
    MenuDistances->Enabled=Pause||EnCours;

    ActionNouveau->Enabled=(!Pause)&&(!EnCours); // v4.4.3
    ActionOuvrir->Enabled=(!Pause)&&(!EnCours); // v4.4.3
	ActionEnregistrer->Enabled=(!Pause)&&(!EnCours); // v4.4.3
	ActionEnregistrerSous->Enabled=(!Pause)&&(!EnCours); // v4.4.3
	ActionVerifierReseau->Enabled=(!Pause)&&(!EnCours);
	ActionMarche->Enabled=!EnCours;
	ActionInterruption->Enabled=EnCours;
	ActionArret->Enabled=EnCours;
	ActionEpidemie->Enabled=(!Pause)&&(!EnCours); // v5.4.2 : Action en mode interrupteur donc désactivé pendant la simulation
	ActionEvolution->Enabled=Pause||EnCours;
    ActionFluiditeTrafic->Enabled=Pause||EnCours;
    ActionStatsParkings->Enabled=Pause||EnCours;
    ActionDirectionHaut->Enabled=(!Pause)&&(!EnCours);
    ActionDirectionDroite->Enabled=(!Pause)&&(!EnCours);
    ActionDirectionBas->Enabled=(!Pause)&&(!EnCours);
    ActionDirectionGauche->Enabled=(!Pause)&&(!EnCours);
    ActionEffacerRoute->Enabled=(!Pause)&&(!EnCours);
    ActionDirectionHautTram->Enabled=(!Pause)&&(!EnCours);
    ActionDirectionDroiteTram->Enabled=(!Pause)&&(!EnCours); // v4.2.2 (Haut répété !)
    ActionDirectionBasTram->Enabled=(!Pause)&&(!EnCours);    // v4.2.2 (Haut répété !)
    ActionDirectionGaucheTram->Enabled=(!Pause)&&(!EnCours); // v4.2.2 (Haut répété !)
    ActionEffacerVoieTram->Enabled=(!Pause)&&(!EnCours);
    ActionToutDroit->Enabled=(!Pause)&&(!EnCours);
    ActionPriorites->Enabled=(!Pause)&&(!EnCours);
    ActionParking->Enabled=(!Pause)&&(!EnCours); // v3.0
    ActionDepotBus->Enabled=(!Pause)&&(!EnCours);
    ActionPassagePietons->Enabled=(!Pause)&&(!EnCours);
    ActionArretBusTram->Enabled=(!Pause)&&(!EnCours); // v3.0
    ActionPassageBus->Enabled=(!Pause)&&(!EnCours);
    ActionPlaceTaxi->Enabled=(!Pause)&&(!EnCours);
    ActionPlacePark->Enabled=(!Pause)&&(!EnCours); // v5.0
    ActionPlaceVehlib->Enabled=(!Pause)&&(!EnCours); // v5.3
    ActionDeplacerFeuSource->Enabled=(!Pause)&&(!EnCours);
    ActionDeplacerFeuDestination->Enabled=(!Pause)&&(!EnCours);
    // Changement d'échelle en abscisse désactivée pendant la simulation (v3.8.3)
    ActionStatsAff10Tours->Enabled=(!Pause)&&(!EnCours);
    ActionStatsAff20Tours->Enabled=(!Pause)&&(!EnCours);
    ActionStatsAff50Tours->Enabled=(!Pause)&&(!EnCours);
    ActionStatsAff100Tours->Enabled=(!Pause)&&(!EnCours);
	ActionStatsAff200Tours->Enabled=(!Pause)&&(!EnCours);
    ActionStatsAff500Tours->Enabled=(!Pause)&&(!EnCours);
    // Décors (v4.2.2)
    ActionNatureTrottoir->Enabled=(!Pause)&&(!EnCours);
    ActionNatureImmeuble->Enabled=(!Pause)&&(!EnCours);
    ActionDecorsArbre->Enabled=(!Pause)&&(!EnCours);
    ActionDecorsPelouse->Enabled=(!Pause)&&(!EnCours);

    if (SimulEnCours)
     {
      TypeDemande=tdRien;
      TimerSimul->Interval=(unsigned)(0.5+1000.0/(cv->NbToursParSeconde));
      TimerSimul->Enabled=true;
     }
    InitialiseDeplacementFeu(); // v3.5
   };
  void ChargeFiche(); // portage : chargement du .dfm (setupUi, liaison, gestionnaires), puis OnCreate
  bool SimulEstModifiee() { return SimulModifiee; };
  void ModifieSimul(bool Modifiee)
   {
    SimulModifiee=Modifiee;
    if (Modifiee)
     {
      if (Caption[Caption.Length()]!='*')
       Caption=Caption+"*";
     }
    else
     if (Caption[Caption.Length()]=='*')
      Caption=Caption.SubString(1, Caption.Length()-1);
   };
  bool SimulEstSanglante() { return ActionSanglante->Checked; }; // v4.3
  void ModifieModeSanglant(bool Sanglant) { ActionSanglante->Checked=Sanglant; }; // v4.3
  bool ResurrectionEstDemandee() { return ActionResurrection->Checked; }; // v4.3.1
  void ModifieDemandeResurrection(bool Resurrection) // v4.3.1
   {
	ActionResurrection->Checked=Resurrection;
	ActionResurrection->Enabled=!ActionResurrection->Checked;
   };
  bool EpidemieEstActive() { return ActionEpidemie->Checked; }; // v5.4.1
  bool ConfirmeSuppressionFeuxUtilisesDansSelection(type_voie TypeVoie);
  void SupprimeFeuxUtilisesDansSelection(type_voie TypeVoie);
  bool ConfirmeSuppressionArretsUtilisesDansSelection(type_voie TypeVoie); // v3.5
  void SupprimeArretsUtilisesDansSelection(type_voie TypeVoie); // v3.5
  void ReinitialiseToutDansSelection(type_voie TypeVoie);
  bool ModifiePassagePietonsSiNecessaire(int x, int y); // v2.0
  void AfficheStatsAttenteTrafic(); // v2.2.7
  direction DirectionOrigineCurseur(int x, int y, direction DirDest); // v3.5
  void RafraichitActionsEdition(mode_edition ModeEdition); // v3.5
  void InitialiseDeplacementFeu(); // v3.5
  void RafraichitFonctionDeplacementFeu(); // v3.5
  void DeplaceFeu(); // v3.8.3
public:
    type_demande TypeDemande;
	Propriete<bool> SimulationEnCours; // portage : __property {read=SimulEstEnCours, write=MarcheArretSimul}
	Propriete<bool> SimulationModifiee; // portage : __property {read=SimulEstModifiee, write=ModifieSimul}
	Propriete<bool> SimulationSanglante; // v4.3 // portage : __property {read=SimulEstSanglante, write=ModifieModeSanglant}
	Propriete<bool> ResurrectionDemandee; // v4.3.1 // portage : __property {read=ResurrectionEstDemandee, write=ModifieDemandeResurrection}
	Propriete<bool> EpidemieActivee; // v5.4.1 // portage : __property {read=EpidemieEstActive}
	bool SelectionFeuxDmd,
         SelectionFeuxAtt,
         SelectionFeuxPtn, // v5.2
         StatsAttenteTrafic, // v2.2.7
         Pause;
    TfrmSimulation(TComponent* Owner);
    ~TfrmSimulation() override; // portage : OnDestroy
    IVueSimulation *VueMoteur; // portage : implémentation d'IVueSimulation (vue du moteur)
    void OuvreFichierDemande(const String &asNomFichier); // portage : QFileOpenEvent (macOS)
    void RedessineCase(int x, int y, bool EffaceFond);
    void AfficheBilan();
	bool SupprimeToutDansSelection(type_voie TypeVoie);
	void AffecteGlyphe(const int NumGlyphe, TBitmap *Glyphe)
	 { // v5.4 pour affecter les BitBtn->Glyph à partir d'une virtualImageList et supporter le HDPI
      VirtualImageListGlyphes->GetBitmap(NumGlyphe, Glyphe);
	 };
};
//---------------------------------------------------------------------------
extern TfrmSimulation *frmSimulation; // portage : PACKAGE
// v3.5 : Ajout de Tram aux constantes de Couleur
extern TColor CoulLigneBusTram[NBMAXLIGNESBUSTRAM];
extern TColor CoulTLigneBusTram[NBMAXLIGNESBUSTRAM];
extern const AnsiString asMemInsuff, asErrInattendue, asFormatCoord;
//---------------------------------------------------------------------------
#endif
