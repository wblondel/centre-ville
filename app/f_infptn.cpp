//---------------------------------------------------------------------------
// f_infptn.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_infptn.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_infptn.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_infptn.h"
#include "ui_f_infptn.h"
#include "f_infptn_composants.h"
//---------------------------------------------------------------------------
TfrmInfosPieton *frmInfosPieton;
//---------------------------------------------------------------------------
TfrmInfosPieton::TfrmInfosPieton(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmInfosPieton::ChargeFiche()
 {
  Ui::frmInfosPieton().setupUi(this);
  LieurComposants Lieur(this, "f_infptn", Composants);
  COMPOSANTS_frmInfosPieton(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmInfosPieton::RafraichitInfos() {}
//---------------------------------------------------------------------------
