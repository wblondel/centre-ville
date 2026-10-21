//---------------------------------------------------------------------------
// f_prmtram.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_prmtram.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_prmtram.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_prmtram.h"
#include "ui_f_prmtram.h"
#include "f_prmtram_composants.h"
//---------------------------------------------------------------------------
TfrmParamTram *frmParamTram;
//---------------------------------------------------------------------------
TfrmParamTram::TfrmParamTram(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  NumArretTramAMontrer=0;
 }
//---------------------------------------------------------------------------
void TfrmParamTram::ChargeFiche()
 {
  Ui::frmParamTram().setupUi(this);
  LieurComposants Lieur(this, "f_prmtram", Composants);
  COMPOSANTS_frmParamTram(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
