//---------------------------------------------------------------------------
// f_prmtaxi.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_prmtaxi.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_prmtaxi.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_prmtaxi.h"
#include "ui_f_prmtaxi.h"
#include "f_prmtaxi_composants.h"
//---------------------------------------------------------------------------
TfrmParamTaxi *frmParamTaxi;
//---------------------------------------------------------------------------
TfrmParamTaxi::TfrmParamTaxi(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmParamTaxi::ChargeFiche()
 {
  Ui::frmParamTaxi().setupUi(this);
  LieurComposants Lieur(this, "f_prmtaxi", Composants);
  COMPOSANTS_frmParamTaxi(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
