//---------------------------------------------------------------------------
// f_afdistfp.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_afdistfp.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_afdistfp.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_afdistfp.h"
#include "ui_f_afdistfp.h"
#include "f_afdistfp_composants.h"
//---------------------------------------------------------------------------
TfrmAffDistFilesPark *frmAffDistFilesPark;
//---------------------------------------------------------------------------
TfrmAffDistFilesPark::TfrmAffDistFilesPark(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmAffDistFilesPark::ChargeFiche()
 {
  Ui::frmAffDistFilesPark().setupUi(this);
  LieurComposants Lieur(this, "f_afdistfp", Composants);
  COMPOSANTS_frmAffDistFilesPark(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmAffDistFilesPark::EffaceInfos() {}
//---------------------------------------------------------------------------
void TfrmAffDistFilesPark::RafraichitInfos() {}
//---------------------------------------------------------------------------
