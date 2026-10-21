//---------------------------------------------------------------------------
// f_statsv.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_statsv.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_statsv.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_statsv.h"
#include "ui_f_statsv.h"
#include "f_statsv_composants.h"
//---------------------------------------------------------------------------
TfrmStatsVehicules *frmStatsVehicules;
//---------------------------------------------------------------------------
TfrmStatsVehicules::TfrmStatsVehicules(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmStatsVehicules::ChargeFiche()
 {
  Ui::frmStatsVehicules().setupUi(this);
  LieurComposants Lieur(this, "f_statsv", Composants);
  COMPOSANTS_frmStatsVehicules(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmStatsVehicules::RafraichitInfos() {}
//---------------------------------------------------------------------------
