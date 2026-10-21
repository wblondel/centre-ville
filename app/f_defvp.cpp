//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "base.h"
#include "f_defvp.h"
#include "f_defv.h"
#include "f_simul.h"
#include "f_deffp.h"
//---------------------------------------------------------------------------
#include "ui_f_defvp.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_defvp_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmDefVoiePietons *frmDefVoiePietons=NULL;
//---------------------------------------------------------------------------
TfrmDefVoiePietons::TfrmDefVoiePietons(TComponent* Owner) : TForm(Owner)
 { ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
  // v5.4 on charge les glyphes manuellement pour prendre en compte le HDPI
  frmSimulation->AffecteGlyphe(GLYPHE_DEFFP, SpeedButtonN->Glyph);
  frmSimulation->AffecteGlyphe(GLYPHE_DEFFP, SpeedButtonE->Glyph);
  frmSimulation->AffecteGlyphe(GLYPHE_DEFFP, SpeedButtonS->Glyph);
  frmSimulation->AffecteGlyphe(GLYPHE_DEFFP, SpeedButtonO->Glyph);
 }
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::DefinitFeu(direction Dir)
 {
  frmDefFeuxPietons->Direction=Dir;
  frmDefFeuxPietons->x=x;
  frmDefFeuxPietons->y=y;
  frmDefFeuxPietons->Show();
 }
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::AppliqueChoix()
 {
  AnsiString asMsg;
  frmSimulation->SimulationModifiee=true;

  // 1. On supprime les feux si on change de priorité (et on ferme la fiche DefFeuxPieton)
  for(int d=nord; d<=ouest; d++)
   if (v->NumFeuP[d-1]&&(!SpeedButton[d-1]->Down))
    {
     cv->SupprimeFeuPietons(v->NumFeuP[d-1]-1);
     if (frmDefFeuxPietons->Visible) frmDefFeuxPietons->Hide();
    }

  // 2. On met à jour l'accessibilité des boutons et groupes
  ButtonAppliquer->Enabled=false;
  for(int d=nord; d<=ouest; d++)
   SpeedButton[d-1]->Down=(v->NumFeuP[d-1]>0);

  // 3. On rafraîchit la scène
  frmSimulation->RedessineCase(x, y, true);
  for(int d=nord; d<=ouest; d++)
   if ((xfo[d-1]!=-1)&&(yfo[d-1]!=-1))
    frmSimulation->RedessineCase( xfo[d-1], yfo[d-1], true);
 }
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::RafraichitInfos()
 {
  if (frmDefFeuxPietons->Visible) frmDefFeuxPietons->Hide();
  x=frmSimulation->DrawGridSimul->Col;
  y=frmSimulation->DrawGridSimul->Row;
  v=&(cv->v[x][y]);

  // On sauvegarde les coordonnées de tous les feux de piétons opposés
  for(int d=nord; d<=ouest; d++)
   if (v->NumFeuP[d-1])
    {
     feu_pieton *f=cv->FeuP+(v->NumFeuP[d-1]-1);
     xfo[d-1]=f->x[f->IFEUOPPDIR(d)];
     yfo[d-1]=f->y[f->IFEUOPPDIR(d)];
    }
   else
    xfo[d-1]=yfo[d-1]=-1;

  if (v->SensVoieTram||v->DirPoss) // On teste si présence de voies. Si pas le cas on affiche les priorités Piétons !
   {
    frmDefVoiePietons->Hide();
    frmDefVoie->Show();
    return;
   }
  LabelCoordonnees->Caption=Format("%2.2d, %2.2d", ARRAYOFCONST((x, y)));
  for(int d=nord; d<=ouest; d++)
   {
    SpeedButton[d-1]->Enabled=cv->V(x,y,d).PassagePietons&&(NATURE(v)==trottoirs);
    SpeedButton[d-1]->Down=(v->NumFeuP[d-1]>0);
    Image[d-1]->Visible=SpeedButton[d-1]->Enabled;
   }
  ButtonAppliquer->Enabled=false; // v5.2 : En cas de changement de coordonnées (boîte non modale) => Annulation implicite donc Appliquer ne peut être actif.
 }
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::FormShow(TObject *Sender)
{
 if (frmDefVoie&&frmDefVoie->Visible) frmDefVoie->Hide(); // Fenêtres non modales exclusives
 frmSimulation->ActionPrioritesPietons->Checked=true;
 RafraichitInfos();
 for(int i=0; i<NBDIR; i++) FeuExistant[i]=(v->NumFeuP[i]>0);
}
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::ButtonOkClick(TObject *Sender)
{
 AppliqueChoix();
 Hide();
}
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::ButtonAppliquerClick(TObject *Sender)
{
 AppliqueChoix();
}
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::ButtonAnnulerClick(TObject *Sender)
{
 // 1. On supprime les feux si on change de priorité (et on ferme la fiche DefFeuxPieton)
 for(int d=nord; d<=ouest; d++)
  if ((!FeuExistant[d-1])&&v->NumFeuP[d-1])
   {
    cv->SupprimeFeuPietons(v->NumFeuP[d-1]-1);
    if (frmDefFeuxPietons->Visible) frmDefFeuxPietons->Hide();
   }

 // 2. On met à jour l'accessibilité des boutons et groupes
 ButtonAppliquer->Enabled=false;
 for(int d=nord; d<=ouest; d++)
  SpeedButton[d-1]->Down=(v->NumFeuP[d-1]>0);

 // 3. On rafraîchit la scène
 frmSimulation->RedessineCase(x, y, true);
 for(int d=nord; d<=ouest; d++)
  if ((xfo[d-1]!=-1)&&(yfo[d-1]!=-1))
   frmSimulation->RedessineCase( xfo[d-1], yfo[d-1], true);
 ButtonAppliquer->Enabled=false;
 Hide();
}
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::FormHide(TObject *Sender)
{
 try // Voir pourquoi
  {
   if (frmDefFeuxPietons&&frmDefFeuxPietons->Visible) frmDefFeuxPietons->Hide();
   frmSimulation->ActionPrioritesPietons->Checked=false;
  }
 catch(...) {}; // portage : __except(true) (exceptions structurées de Windows) -> catch(...)
}
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::ButtonAideClick(TObject *Sender)
{
 AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::FormCreate(TObject *Sender)
{
 SpeedButton[0]=SpeedButtonN;
 SpeedButton[1]=SpeedButtonE;
 SpeedButton[2]=SpeedButtonS;
 SpeedButton[3]=SpeedButtonO;
 Image[0]=ImageNN;
 Image[1]=ImageEE;
 Image[2]=ImageSS;
 Image[3]=ImageOO;
}
//---------------------------------------------------------------------------
void TfrmDefVoiePietons::SpeedButtonClick(TObject *Sender)
{
 TSpeedButton *SpeedButtonNESO=(TSpeedButton*)Sender;
 direction d=(direction)(TagDe((TComponent*)Sender)+1); // portage : TComponent::Tag -> TagDe
 ButtonAppliquer->Enabled=true;
 if (SpeedButtonNESO->Down)
  {
   if (cv->FeuPietonsPossible(x, y, d))
    DefinitFeu(d);
   else
    {
     AfficheMessage("\
Impossible de définir un feu de piétons ici car le passage piétons est un cul de sac et le feu piétons opposé ne peut être placé.\n\n\
Placez un trottoir à l'extrémité du passage piétons puis réessayez.", asTitreCV, tmErreur, 0, 0);
     SpeedButtonNESO->Down=false;
    }
  }
 else
  {
   if (frmDefFeuxPietons&&frmDefFeuxPietons->Visible)
    frmDefFeuxPietons->Hide();
   SpeedButtonNESO->Down=false;
  }
}
//---------------------------------------------------------------------------


void TfrmDefVoiePietons::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112)
   AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmDefVoiePietons::Gestionnaire);
void TfrmDefVoiePietons::ChargeFiche()
 {
  Ui::frmDefVoiePietons().setupUi(this);
  LieurComposants Lieur(this, "f_defvp", Composants);
  COMPOSANTS_frmDefVoiePietons(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmDefVoiePietons(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------
