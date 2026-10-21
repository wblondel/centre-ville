//---------------------------------------------------------------------------
// f_statsat.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_statsat.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_statsat.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_statsat.h"
#include "ui_f_statsat.h"
#include "f_statsat_composants.h"
//---------------------------------------------------------------------------
TfrmStatsAttenteTrafic *frmStatsAttenteTrafic;
//---------------------------------------------------------------------------
TfrmStatsAttenteTrafic::TfrmStatsAttenteTrafic(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::ChargeFiche()
 {
  Ui::frmStatsAttenteTrafic().setupUi(this);
  LieurComposants Lieur(this, "f_statsat", Composants);
  COMPOSANTS_frmStatsAttenteTrafic(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
TColor TfrmStatsAttenteTrafic::CouleurCase(int x, int y) { (void)x; (void)y; return clBlack; }
//---------------------------------------------------------------------------
AnsiString TfrmStatsAttenteTrafic::asMsgDetailCase(int x, int y) { (void)x; (void)y; return AnsiString(); }
//---------------------------------------------------------------------------
