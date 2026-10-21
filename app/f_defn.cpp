//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "base.h"
#include "simul.h"
#include "f_simul.h"
#include "f_defn.h"
//---------------------------------------------------------------------------
#include "ui_f_defn.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_defn_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmDefNature *frmDefNature=NULL;
//---------------------------------------------------------------------------
TfrmDefNature::TfrmDefNature(TComponent* Owner)
  : TForm(Owner)
{ ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
 // v5.4 cette boîte ne s'affiche pas comme il faut en HDPI :^(
 // portage : Qt met déjà la fiche à l'échelle de l'écran (pixels logiques) : l'agrandir
 // encore la rendrait deux fois trop grande sur un écran Retina
 //Width=Width*Screen->PixelsPerInch/96;
 //Height=Height*Screen->PixelsPerInch/96;
}
//---------------------------------------------------------------------------
void TfrmDefNature::RadioGroupNatureClick(TObject *Sender)
{
 if (RadioGroupNature->ItemIndex==1)
  CheckBoxPelouse->Enabled=true;
 else
  {
   CheckBoxPelouse->Checked=false;
   CheckBoxPelouse->Enabled=false;
  }
 ButtonAppliquer->Enabled=(RadioGroupNature->ItemIndex>-1);
}
//---------------------------------------------------------------------------
void TfrmDefNature::ButtonAppliquerClick(TObject *Sender)
{
 bool BordGauche, BordDroit, BordHaut, BordBas;
 for(int x=frmSimulation->DrawGridSimul->Selection.Left; x<=frmSimulation->DrawGridSimul->Selection.Right; x++)
  for(int y=frmSimulation->DrawGridSimul->Selection.Top; y<=frmSimulation->DrawGridSimul->Selection.Bottom; y++)
   {
    cv->v[x][y].DefinitNature( (nature)( trottoirs*RadioGroupNature->ItemIndex+
                                         CheckBoxPelouse->Checked*(pelouse-trottoirs)),
                               CheckBoxEcraser->Checked);
    BordGauche=(x==frmSimulation->DrawGridSimul->Selection.Left);
    BordDroit=(x==frmSimulation->DrawGridSimul->Selection.Right);
    BordHaut=(y==frmSimulation->DrawGridSimul->Selection.Top);
    BordBas=(y==frmSimulation->DrawGridSimul->Selection.Bottom);
    if (BordDroit||BordHaut) cv->v[(x+BordDroit)%cv->NbX][(y+BordHaut*(cv->NbY-1))%cv->NbY].RedessineCase(true);
    if (BordDroit||BordBas) cv->v[(x+BordDroit)%cv->NbX][(y+BordBas)%cv->NbY].RedessineCase(true);
    if (BordGauche||BordBas) cv->v[(x+BordGauche*(cv->NbX-1))%cv->NbX][(y+BordBas)%cv->NbY].RedessineCase(true);
    if (BordGauche||BordHaut) cv->v[(x+BordGauche*(cv->NbX-1))%cv->NbX][(y+BordHaut*(cv->NbY-1))%cv->NbY].RedessineCase(true);
    if (BordDroit||BordBas) cv->v[(x+BordDroit)%cv->NbX][(y+BordBas)%cv->NbY].RedessineCase(true);
    if (BordDroit||BordHaut) cv->v[(x+BordDroit)%cv->NbX][(y+BordHaut*(cv->NbY-1))%cv->NbY].RedessineCase(true);
    if (BordGauche||BordBas) cv->v[(x+BordGauche*(cv->NbX-1))%cv->NbX][(y+BordBas)%cv->NbY].RedessineCase(true);
    if (BordGauche||BordHaut) cv->v[(x+BordGauche*(cv->NbX-1))%cv->NbX][(y+BordHaut*(cv->NbY-1))%cv->NbY].RedessineCase(true);
   }
 }
//---------------------------------------------------------------------------
void TfrmDefNature::ButtonAideClick(TObject *Sender)
{
 AfficheAideContextuelle(this); // v3.8
}
//---------------------------------------------------------------------------
void TfrmDefNature::FormShow(TObject *Sender)
{
 frmSimulation->ActionDefinirNature->Checked=true;
 RadioGroupNature->ItemIndex=-1;
 CheckBoxPelouse->Enabled=false;
 CheckBoxPelouse->Checked=false;
 ButtonAppliquer->Enabled=false;
}
//---------------------------------------------------------------------------
void TfrmDefNature::FormHide(TObject *Sender)
{
 frmSimulation->ActionDefinirNature->Checked=false;
}
//---------------------------------------------------------------------------
void TfrmDefNature::ButtonFermerClick(TObject *Sender)
{
 Hide();
}
//---------------------------------------------------------------------------
void TfrmDefNature::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112)
   AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmDefNature::Gestionnaire);
void TfrmDefNature::ChargeFiche()
 {
  Ui::frmDefNature().setupUi(this);
  LieurComposants Lieur(this, "f_defn", Composants);
  COMPOSANTS_frmDefNature(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmDefNature(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------
