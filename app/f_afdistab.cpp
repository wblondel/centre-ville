//---------------------------------------------------------------------------
// f_afdistab.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_afdistab.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_afdistab.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_afdistab.h"
#include "ui_f_afdistab.h"
#include "f_afdistab_composants.h"
//---------------------------------------------------------------------------
TfrmAffDistArretsBus *frmAffDistArretsBus;
//---------------------------------------------------------------------------
TfrmAffDistArretsBus::TfrmAffDistArretsBus(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmAffDistArretsBus::ChargeFiche()
 {
  Ui::frmAffDistArretsBus().setupUi(this);
  LieurComposants Lieur(this, "f_afdistab", Composants);
  COMPOSANTS_frmAffDistArretsBus(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmAffDistArretsBus::EffaceInfos() {}
//---------------------------------------------------------------------------
void TfrmAffDistArretsBus::RafraichitInfos() {}
//---------------------------------------------------------------------------
