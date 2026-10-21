//---------------------------------------------------------------------------
// f_genres.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_genres.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_genres.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_genres.h"
#include "ui_f_genres.h"
#include "f_genres_composants.h"
//---------------------------------------------------------------------------
TfrmGeneration *frmGeneration;
//---------------------------------------------------------------------------
TfrmGeneration::TfrmGeneration(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmGeneration::ChargeFiche()
 {
  Ui::frmGeneration().setupUi(this);
  LieurComposants Lieur(this, "f_genres", Composants);
  COMPOSANTS_frmGeneration(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void GenereReseau()
 {
  if ((frmGeneration=new TfrmGeneration(Application)))
   {
    frmGeneration->ShowModal();
    delete frmGeneration;
    frmGeneration=NULL;
   }
 }
//---------------------------------------------------------------------------
