//---------------------------------------------------------------------------
// f_carfours.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_carfours.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_carfours.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_carfours.h"
#include "f_simul.h"
#include "ui_f_carfours.h"
#include "f_carfours_composants.h"
//---------------------------------------------------------------------------
TfrmCarrefours *frmCarrefours;
//---------------------------------------------------------------------------
TfrmCarrefours::TfrmCarrefours(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmCarrefours::ChargeFiche()
 {
  Ui::frmCarrefours().setupUi(this);
  LieurComposants Lieur(this, "f_carfours", Composants);
  COMPOSANTS_frmCarrefours(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmCarrefours::Active(int Colonnes, int Lignes) { (void)Colonnes; (void)Lignes; }
//---------------------------------------------------------------------------
int Carrefours(int Colonnes, int Lignes)
{
 frmCarrefours->Active(Colonnes, Lignes);
 return frmCarrefours->ShowModal();
}
//---------------------------------------------------------------------------
