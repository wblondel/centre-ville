//---------------------------------------------------------------------------
// f_infbus.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_infbus.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_infbus.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_infbus.h"
#include "ui_f_infbus.h"
#include "f_infbus_composants.h"
//---------------------------------------------------------------------------
TfrmInfosBus *frmInfosBus;
//---------------------------------------------------------------------------
TfrmInfosBus::TfrmInfosBus(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmInfosBus::ChargeFiche()
 {
  Ui::frmInfosBus().setupUi(this);
  LieurComposants Lieur(this, "f_infbus", Composants);
  COMPOSANTS_frmInfosBus(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmInfosBus::RafraichitInfos() {}
//---------------------------------------------------------------------------
