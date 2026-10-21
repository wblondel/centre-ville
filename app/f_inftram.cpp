//---------------------------------------------------------------------------
// f_inftram.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_inftram.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_inftram.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_inftram.h"
#include "ui_f_inftram.h"
#include "f_inftram_composants.h"
//---------------------------------------------------------------------------
TfrmInfosTram *frmInfosTram;
//---------------------------------------------------------------------------
TfrmInfosTram::TfrmInfosTram(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmInfosTram::ChargeFiche()
 {
  Ui::frmInfosTram().setupUi(this);
  LieurComposants Lieur(this, "f_inftram", Composants);
  COMPOSANTS_frmInfosTram(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmInfosTram::RafraichitInfos() {}
//---------------------------------------------------------------------------
