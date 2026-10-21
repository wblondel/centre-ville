//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "f_simul.h"
#include "f_genres.h"
//---------------------------------------------------------------------------
#include "ui_f_genres.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_genres_composants.h" // portage : #pragma resource "*.dfm"
#include "compat/alea.h" // portage : rand() de la RTL Borland (après les en-têtes système)
//---------------------------------------------------------------------------
TfrmGeneration *frmGeneration=NULL; // portage : =NULL explicite
//---------------------------------------------------------------------------
void GenereReseau()
 {
  if ((frmGeneration=new TfrmGeneration(Application)))
   { // portage : try/__finally -> garde RAII (Finalement)
    Finalement f([&]()
    {
     delete frmGeneration;
     frmGeneration=NULL;
    });
     frmGeneration->ShowModal();
   }
 }
//---------------------------------------------------------------------------
TfrmGeneration::TfrmGeneration(TComponent* Owner)
  : TForm(Owner)
{ ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
}
//---------------------------------------------------------------------------
void TfrmGeneration::ButtonOkClick(TObject *Sender)
{
 cv->PrepareGenerationAleatoire( TrackBarTailleMinSegments->Position,
                                 TrackBarTailleMaxSegments->Position,
                                 TrackBarProbaSortieIntersection->Position,
                                 TrackBarProbaEntreeIntersection->Position,
                                 TrackBarProbaTraverseeRoute->Position);
 // portage : l'ordre d'évaluation des arguments n'est pas spécifié en C++ ; le binaire
 // d'origine les évalue de gauche à droite (engine/README.md) : x, puis y, puis la direction
 int x=rand()%cv->NbX, y=rand()%cv->NbY; direction d=(direction)(1+rand()%NBDIR); cv->Genere(x, y, d);
 frmSimulation->DrawGridSimul->Repaint();
}
//---------------------------------------------------------------------------

void TfrmGeneration::ButtonAideClick(TObject *Sender)
{
 AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------

void TfrmGeneration::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112)
   AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmGeneration::Gestionnaire);
void TfrmGeneration::ChargeFiche()
 {
  Ui::frmGeneration().setupUi(this);
  LieurComposants Lieur(this, "f_genres", Composants);
  COMPOSANTS_frmGeneration(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmGeneration(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------
