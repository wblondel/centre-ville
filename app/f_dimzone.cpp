//---------------------------------------------------------------------------
// f_dimzone.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_dimzone.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_dimzone.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_dimzone.h"
#include "ui_f_dimzone.h"
#include "f_dimzone_composants.h"
//---------------------------------------------------------------------------
TfrmDimensionsZone *frmDimensionsZone;
//---------------------------------------------------------------------------
TfrmDimensionsZone::TfrmDimensionsZone(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  NbX=NbY=0;
 }
//---------------------------------------------------------------------------
void TfrmDimensionsZone::ChargeFiche()
 {
  Ui::frmDimensionsZone().setupUi(this);
  LieurComposants Lieur(this, "f_dimzone", Composants);
  COMPOSANTS_frmDimensionsZone(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
bool DefinitDimensionsZone(int &NbX, int &NbY, bool &GenereReseau)
 {
  (void)NbX; (void)NbY; (void)GenereReseau;
  if ((frmDimensionsZone=new TfrmDimensionsZone(Application)))
   {
    frmDimensionsZone->ShowModal();
    delete frmDimensionsZone;
    frmDimensionsZone=NULL;
   }
  return false; // fiche non portée : comme Annuler
 }
//---------------------------------------------------------------------------
