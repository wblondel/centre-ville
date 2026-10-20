//---------------------------------------------------------------------------
// f_statsc_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_statsc.dfm : ne pas modifier
//
// COMPOSANTS_frmStatsComposition(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmStatsComposition(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_statsc_composantsH
#define f_statsc_composantsH

#define COMPOSANTS_frmStatsComposition(X) \
  X(TLabel, Label1) \
  X(TLabel, Label2) \
  X(TLabel, Label3) \
  X(TLabel, Label4) \
  X(TLabel, Label5) \
  X(TLabel, LabelTotal) \
  X(TLabel, LabelTotalPC) \
  X(TLabel, LabelRoutes) \
  X(TLabel, LabelRoutesPC) \
  X(TLabel, LabelPrioritesR) \
  X(TLabel, LabelPrioritesRPC) \
  X(TLabel, Label6) \
  X(TLabel, Label8) \
  X(TLabel, Label9) \
  X(TLabel, Label10) \
  X(TLabel, Label11) \
  X(TLabel, LabelFeuxRMax) \
  X(TLabel, LabelFeuxRReste) \
  X(TLabel, LabelFeuxR) \
  X(TLabel, LabelFeuxRPC) \
  X(TLabel, LabelClpR) \
  X(TLabel, LabelClpRPC) \
  X(TLabel, LabelStopsR) \
  X(TLabel, LabelStopsRPC) \
  X(TLabel, Label24) \
  X(TLabel, LabelDir) \
  X(TLabel, LabelDirPC) \
  X(TLabel, Label27) \
  X(TLabel, Label28) \
  X(TLabel, Label29) \
  X(TLabel, LabelNord) \
  X(TLabel, LabelEst) \
  X(TLabel, LabelSud) \
  X(TLabel, LabelSudPC) \
  X(TLabel, LabelEstPC) \
  X(TLabel, LabelNordPC) \
  X(TLabel, Label36) \
  X(TLabel, LabelOuest) \
  X(TLabel, LabelOuestPC) \
  X(TLabel, Label12) \
  X(TLabel, LabelParkings) \
  X(TLabel, LabelParkingsPC) \
  X(TLabel, LabelParkingsMax) \
  X(TLabel, LabelParkingsReste) \
  X(TLabel, Label7) \
  X(TLabel, Label14) \
  X(TLabel, Label15) \
  X(TLabel, Label16) \
  X(TLabel, Label17) \
  X(TLabel, Label18) \
  X(TLabel, Label19) \
  X(TLabel, Label20) \
  X(TLabel, Label21) \
  X(TLabel, Label22) \
  X(TLabel, Label23) \
  X(TLabel, Label13) \
  X(TLabel, LabelArretsBus) \
  X(TLabel, LabelArretsBusPC) \
  X(TLabel, Label30) \
  X(TLabel, LabelArretsBusMax) \
  X(TLabel, LabelArretsBusReste) \
  X(TLabel, Label25) \
  X(TLabel, LabelArretsTram) \
  X(TLabel, LabelArretsTramPC) \
  X(TLabel, Label32) \
  X(TLabel, LabelArretsTramMax) \
  X(TLabel, LabelArretsTramReste) \
  X(TLabel, Label26) \
  X(TLabel, LabelVoiesTram) \
  X(TLabel, LabelVoiesTramPC) \
  X(TLabel, Label31) \
  X(TLabel, Label33) \
  X(TLabel, Label34) \
  X(TBevel, Bevel1) \
  X(TBevel, Bevel2) \
  X(TLabel, Label35) \
  X(TLabel, LabelPrioritesV) \
  X(TLabel, LabelPrioritesVPC) \
  X(TLabel, Label39) \
  X(TLabel, Label40) \
  X(TLabel, Label41) \
  X(TLabel, LabelFeuxVMax) \
  X(TLabel, LabelFeuxVReste) \
  X(TLabel, LabelFeuxV) \
  X(TLabel, LabelFeuxVPC) \
  X(TLabel, LabelClpV) \
  X(TLabel, LabelClpVPC) \
  X(TLabel, LabelStopsV) \
  X(TLabel, LabelStopsVPC) \
  X(TLabel, Label50) \
  X(TLabel, Label51) \
  X(TLabel, Label52) \
  X(TLabel, Label53) \
  X(TLabel, Label37) \
  X(TLabel, Label38) \
  X(TLabel, Label42) \
  X(TLabel, Label43) \
  X(TLabel, Label44) \
  X(TLabel, Label45) \
  X(TLabel, LabelPlacesTaxi) \
  X(TLabel, Label47) \
  X(TLabel, LabelPlacesTaxiPC) \
  X(TLabel, LabelPlacesTaxiMax) \
  X(TLabel, LabelPlacesTaxiReste) \
  X(TLabel, Label46) \
  X(TLabel, LabelPlacesPark) \
  X(TLabel, LabelPlacesParkPC) \
  X(TLabel, Label54) \
  X(TLabel, LabelPlacesParkMax) \
  X(TLabel, LabelPlacesParkReste) \
  X(TLabel, Label48) \
  X(TLabel, Label49) \
  X(TLabel, Label55) \
  X(TButton, ButtonOk) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmStatsComposition(X) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
