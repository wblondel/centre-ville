//---------------------------------------------------------------------------
// f_statsc.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_statsc.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_statsc.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_statsc.h"
#include "ui_f_statsc.h"
#include "f_statsc_composants.h"
//---------------------------------------------------------------------------
TfrmStatsComposition *frmStatsComposition;
//---------------------------------------------------------------------------
TfrmStatsComposition::TfrmStatsComposition(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmStatsComposition::ChargeFiche()
 {
  Ui::frmStatsComposition().setupUi(this);
  LieurComposants Lieur(this, "f_statsc", Composants);
  COMPOSANTS_frmStatsComposition(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void Composition()
 {
  if ((frmStatsComposition=new TfrmStatsComposition(Application)))
   {
    frmStatsComposition->ShowModal();
    frmStatsComposition->Release();
    frmStatsComposition=NULL;
   }
 }
//---------------------------------------------------------------------------
