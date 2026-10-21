//---------------------------------------------------------------------------
// f_stats.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_stats.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_stats.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_stats.h"
#include "ui_f_stats.h"
#include "f_stats_composants.h"
//---------------------------------------------------------------------------
TfrmStatistiques *frmStatistiques;
//---------------------------------------------------------------------------
TfrmStatistiques::TfrmStatistiques(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  AffichageEpidemique=false;
 }
//---------------------------------------------------------------------------
void TfrmStatistiques::ChargeFiche()
 {
  Ui::frmStatistiques().setupUi(this);
  LieurComposants Lieur(this, "f_stats", Composants);
  COMPOSANTS_frmStatistiques(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
