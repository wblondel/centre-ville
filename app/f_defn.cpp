//---------------------------------------------------------------------------
// f_defn.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_defn.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_defn.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_defn.h"
#include "ui_f_defn.h"
#include "f_defn_composants.h"
//---------------------------------------------------------------------------
TfrmDefNature *frmDefNature=NULL;
//---------------------------------------------------------------------------
TfrmDefNature::TfrmDefNature(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmDefNature::ChargeFiche()
 {
  Ui::frmDefNature().setupUi(this);
  LieurComposants Lieur(this, "f_defn", Composants);
  COMPOSANTS_frmDefNature(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
