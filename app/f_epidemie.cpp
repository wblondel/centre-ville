//---------------------------------------------------------------------------
// f_epidemie.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_epidemie.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_epidemie.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_epidemie.h"
#include "ui_f_epidemie.h"
#include "f_epidemie_composants.h"
//---------------------------------------------------------------------------
TfrmEpidemie *frmEpidemie;
//---------------------------------------------------------------------------
TfrmEpidemie::TfrmEpidemie(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmEpidemie::ChargeFiche()
 {
  Ui::frmEpidemie().setupUi(this);
  LieurComposants Lieur(this, "f_epidemie", Composants);
  COMPOSANTS_frmEpidemie(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
