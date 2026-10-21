//---------------------------------------------------------------------------
// f_afdistft.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_afdistft.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_afdistft.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_afdistft.h"
#include "ui_f_afdistft.h"
#include "f_afdistft_composants.h"
//---------------------------------------------------------------------------
TfrmAffDistFilesTaxi *frmAffDistFilesTaxi;
//---------------------------------------------------------------------------
TfrmAffDistFilesTaxi::TfrmAffDistFilesTaxi(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmAffDistFilesTaxi::ChargeFiche()
 {
  Ui::frmAffDistFilesTaxi().setupUi(this);
  LieurComposants Lieur(this, "f_afdistft", Composants);
  COMPOSANTS_frmAffDistFilesTaxi(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmAffDistFilesTaxi::EffaceInfos() {}
//---------------------------------------------------------------------------
void TfrmAffDistFilesTaxi::RafraichitInfos() {}
//---------------------------------------------------------------------------
