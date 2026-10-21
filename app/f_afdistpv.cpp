//---------------------------------------------------------------------------
// f_afdistpv.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_afdistpv.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_afdistpv.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_afdistpv.h"
#include "ui_f_afdistpv.h"
#include "f_afdistpv_composants.h"
//---------------------------------------------------------------------------
TfrmAffDistPlacesVehlib *frmAffDistPlacesVehlib;
//---------------------------------------------------------------------------
TfrmAffDistPlacesVehlib::TfrmAffDistPlacesVehlib(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmAffDistPlacesVehlib::ChargeFiche()
 {
  Ui::frmAffDistPlacesVehlib().setupUi(this);
  LieurComposants Lieur(this, "f_afdistpv", Composants);
  COMPOSANTS_frmAffDistPlacesVehlib(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmAffDistPlacesVehlib::EffaceInfos() {}
//---------------------------------------------------------------------------
void TfrmAffDistPlacesVehlib::RafraichitInfos() {}
//---------------------------------------------------------------------------
