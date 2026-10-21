//---------------------------------------------------------------------------
// f_defv.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_defv.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_defv.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_defv.h"
#include "ui_f_defv.h"
#include "f_defv_composants.h"
//---------------------------------------------------------------------------
TfrmDefVoie *frmDefVoie=NULL;
//---------------------------------------------------------------------------
TfrmDefVoie::TfrmDefVoie(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmDefVoie::ChargeFiche()
 {
  Ui::frmDefVoie().setupUi(this);
  LieurComposants Lieur(this, "f_defv", Composants);
  COMPOSANTS_frmDefVoie(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmDefVoie::RafraichitInfos() {}
//---------------------------------------------------------------------------
void TfrmDefVoie::AppliqueChoix() {}
//---------------------------------------------------------------------------
