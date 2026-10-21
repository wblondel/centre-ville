//---------------------------------------------------------------------------
// f_infveh.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_infveh.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_infveh.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_infveh.h"
#include "ui_f_infveh.h"
#include "f_infveh_composants.h"
//---------------------------------------------------------------------------
TfrmInfosVehicule *frmInfosVehicule;
//---------------------------------------------------------------------------
TfrmInfosVehicule::TfrmInfosVehicule(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmInfosVehicule::ChargeFiche()
 {
  Ui::frmInfosVehicule().setupUi(this);
  LieurComposants Lieur(this, "f_infveh", Composants);
  COMPOSANTS_frmInfosVehicule(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmInfosVehicule::RafraichitInfos() {}
//---------------------------------------------------------------------------
