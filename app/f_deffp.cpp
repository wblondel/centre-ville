//---------------------------------------------------------------------------
// f_deffp.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_deffp.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_deffp.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_deffp.h"
#include "ui_f_deffp.h"
#include "f_deffp_composants.h"
//---------------------------------------------------------------------------
TfrmDefFeuxPietons *frmDefFeuxPietons=NULL;
//---------------------------------------------------------------------------
TfrmDefFeuxPietons::TfrmDefFeuxPietons(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  NbFeux=0; NumFeux=NULL; x=y=0; Direction=indefinie;
 }
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::ChargeFiche()
 {
  Ui::frmDefFeuxPietons().setupUi(this);
  LieurComposants Lieur(this, "f_deffp", Composants);
  COMPOSANTS_frmDefFeuxPietons(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
bool TfrmDefFeuxPietons::RafraichitInfos() { return true; }
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::RafraichitFeuxPietons() {}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::AnalyseEtatFeux() {}
//---------------------------------------------------------------------------
bool TfrmDefFeuxPietons::AppliqueChoix() { return true; }
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::RafraichitInfosFeux() {}
//---------------------------------------------------------------------------
