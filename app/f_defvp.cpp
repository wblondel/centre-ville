//---------------------------------------------------------------------------
// f_defvp.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_defvp.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_defvp.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_defvp.h"
#include "ui_f_defvp.h"
#include "f_defvp_composants.h"
//---------------------------------------------------------------------------
TfrmDefVoiePietons *frmDefVoiePietons=NULL;
//---------------------------------------------------------------------------
TfrmDefVoiePietons::TfrmDefVoiePietons(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::ChargeFiche()
 {
  Ui::frmDefVoiePietons().setupUi(this);
  LieurComposants Lieur(this, "f_defvp", Composants);
  COMPOSANTS_frmDefVoiePietons(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::RafraichitInfos() {}
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::AppliqueChoix() {}
//---------------------------------------------------------------------------
