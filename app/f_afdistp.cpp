//---------------------------------------------------------------------------
// f_afdistp.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_afdistp.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_afdistp.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_afdistp.h"
#include "ui_f_afdistp.h"
#include "f_afdistp_composants.h"
//---------------------------------------------------------------------------
TfrmAffDistParkings *frmAffDistParkings;
//---------------------------------------------------------------------------
TfrmAffDistParkings::TfrmAffDistParkings(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmAffDistParkings::ChargeFiche()
 {
  Ui::frmAffDistParkings().setupUi(this);
  LieurComposants Lieur(this, "f_afdistp", Composants);
  COMPOSANTS_frmAffDistParkings(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmAffDistParkings::EffaceInfos() {}
//---------------------------------------------------------------------------
void TfrmAffDistParkings::RafraichitInfos() {}
//---------------------------------------------------------------------------
