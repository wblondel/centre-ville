//---------------------------------------------------------------------------
// f_prmvehlib.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_prmvehlib.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_prmvehlib.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_prmvehlib.h"
#include "ui_f_prmvehlib.h"
#include "f_prmvehlib_composants.h"
//---------------------------------------------------------------------------
TfrmParamVehlib *frmParamVehlib;
//---------------------------------------------------------------------------
TfrmParamVehlib::TfrmParamVehlib(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmParamVehlib::ChargeFiche()
 {
  Ui::frmParamVehlib().setupUi(this);
  LieurComposants Lieur(this, "f_prmvehlib", Composants);
  COMPOSANTS_frmParamVehlib(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
