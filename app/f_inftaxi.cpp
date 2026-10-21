//---------------------------------------------------------------------------
// f_inftaxi.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_inftaxi.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_inftaxi.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_inftaxi.h"
#include "ui_f_inftaxi.h"
#include "f_inftaxi_composants.h"
//---------------------------------------------------------------------------
TfrmInfosTaxi *frmInfosTaxi;
//---------------------------------------------------------------------------
TfrmInfosTaxi::TfrmInfosTaxi(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmInfosTaxi::ChargeFiche()
 {
  Ui::frmInfosTaxi().setupUi(this);
  LieurComposants Lieur(this, "f_inftaxi", Composants);
  COMPOSANTS_frmInfosTaxi(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmInfosTaxi::RafraichitInfos() {}
//---------------------------------------------------------------------------
