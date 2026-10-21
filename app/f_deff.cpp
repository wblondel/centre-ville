//---------------------------------------------------------------------------
// f_deff.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_deff.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_deff.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_deff.h"
#include "ui_f_deff.h"
#include "f_deff_composants.h"
//---------------------------------------------------------------------------
TfrmDefFeux *frmDefFeux=NULL;
//---------------------------------------------------------------------------
TfrmDefFeux::TfrmDefFeux(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  NbFeuxDmd=0; NbFeuxAtt=0; NumFeuxDmd=NULL; NumFeuxAtt=NULL; x=y=0; Direction=indefinie;
 }
//---------------------------------------------------------------------------
void TfrmDefFeux::ChargeFiche()
 {
  Ui::frmDefFeux().setupUi(this);
  LieurComposants Lieur(this, "f_deff", Composants);
  COMPOSANTS_frmDefFeux(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
bool TfrmDefFeux::RafraichitInfos() { return true; }
//---------------------------------------------------------------------------
bool TfrmDefFeux::AppliqueChoix() { return true; }
//---------------------------------------------------------------------------
void TfrmDefFeux::RafraichitInfosFeuxDmd() {}
//---------------------------------------------------------------------------
void TfrmDefFeux::RafraichitInfosFeuxAtt() {}
//---------------------------------------------------------------------------
