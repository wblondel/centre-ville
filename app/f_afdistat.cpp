//---------------------------------------------------------------------------
// f_afdistat.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_afdistat.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_afdistat.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_afdistat.h"
#include "ui_f_afdistat.h"
#include "f_afdistat_composants.h"
//---------------------------------------------------------------------------
TfrmAffDistArretsTram *frmAffDistArretsTram;
//---------------------------------------------------------------------------
TfrmAffDistArretsTram::TfrmAffDistArretsTram(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmAffDistArretsTram::ChargeFiche()
 {
  Ui::frmAffDistArretsTram().setupUi(this);
  LieurComposants Lieur(this, "f_afdistat", Composants);
  COMPOSANTS_frmAffDistArretsTram(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmAffDistArretsTram::EffaceInfos() {}
//---------------------------------------------------------------------------
void TfrmAffDistArretsTram::RafraichitInfos() {}
//---------------------------------------------------------------------------
