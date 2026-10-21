//---------------------------------------------------------------------------
// f_prmbus.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_prmbus.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_prmbus.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_prmbus.h"
#include "ui_f_prmbus.h"
#include "f_prmbus_composants.h"
//---------------------------------------------------------------------------
TfrmParamBus *frmParamBus;
//---------------------------------------------------------------------------
TfrmParamBus::TfrmParamBus(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  NumArretBusAMontrer=0;
 }
//---------------------------------------------------------------------------
void TfrmParamBus::ChargeFiche()
 {
  Ui::frmParamBus().setupUi(this);
  LieurComposants Lieur(this, "f_prmbus", Composants);
  COMPOSANTS_frmParamBus(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
