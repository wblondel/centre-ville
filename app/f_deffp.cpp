//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "f_simul.h"
#include "f_deffp.h"
#include "f_defvp.h"
//---------------------------------------------------------------------------
#include "ui_f_deffp.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
//#pragma link "cspin" // portage : TCSpinEdit dans vcl/controles.h
#include "f_deffp_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmDefFeuxPietons *frmDefFeuxPietons=NULL;
//---------------------------------------------------------------------------
TfrmDefFeuxPietons::TfrmDefFeuxPietons(TComponent* Owner) : TForm(Owner)
 { ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
  NbFeux=0;
  NumFeux=NULL;
  // v5.4 on charge les glyphes manuellement pour prendre en compte le HDPI
  frmSimulation->AffecteGlyphe(GLYPHE_SELF, SpeedButtonSelFeux->Glyph);
 }
//---------------------------------------------------------------------------
bool TfrmDefFeuxPietons::RafraichitInfos()
 {
  int i;
  v=&(cv->v[x][y]);
  Numero=v->NumFeuP[Direction-1];
  if (!Numero)
   Numero=cv->AjouteFeuPietons(x, y, Direction);
  v->NumFeuP[Direction-1]=Numero;
  f=&(cv->FeuP[Numero-1]);
  LabelCoordonnees->Caption=Format("%2.2d, %2.2d", ARRAYOFCONST((x, y)));
  LabelDirection->Caption=asDir[Direction];
  if (Numero) LabelNumero->Caption=IntToStr(Numero);
  RadioGroupEtat->ItemIndex=f->Etat-(f->Etat==rouge);
  RadioGroupTypeDmdVert->ItemIndex=f->TypeDmdVert;
  RadioGroupTypePsgRouge->ItemIndex=f->TypePsgRouge;
  CSpinEditDureeVert->Value=f->DureeVert;
  CSpinEditDureeRouge->Value=f->DureeRouge;
  LabelDureeTraversee->Caption=IntToStr(f->DureeTraversee); // portage : conversion implicite int -> AnsiString de C++Builder
  // Copie des listes de feux
  NbFeux=f->NbFeux;
  if (NumFeux) { delete[] NumFeux; NumFeux=NULL; }
  if ((NbFeux)&&(!(NumFeux=new int[NbFeux])))
    {
     AfficheMessage(asMemInsuff,asErrInattendue, tmErreur, 0, 0);
     return false;
    }
  for(i=0; i<NbFeux; i++) NumFeux[i]=f->NumFeux[i];
  // Affichage du nombre de feux de la liste
  LabelNbFeux->Caption=IntToStr(NbFeux);
  LabelCoordonneesFeuPietonsOppose->Caption=Format("%2.2d, %2.2d",
                                                   ARRAYOFCONST((f->x[f->IFEUOPPDIR(Direction)],
                                                                 f->y[f->IFEUOPPDIR(Direction)])));
  ButtonAppliquer->Enabled=false; // v5.2 : En cas de changement de coordonnées (boîte non modale) => Annulation implicite donc Appliquer ne peut être actif.
  return true;
 }
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::RafraichitFeuxPietons()
 {
  frmSimulation->RedessineCase(x, y, true);
  frmSimulation->RedessineCase( f->x[f->IFEUOPPDIR(Direction)],
                                f->y[f->IFEUOPPDIR(Direction)],
                                true);
 }
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::AnalyseEtatFeux() // v5.2.1
 {
  etat_feu EtatFP=(etat_feu)(RadioGroupEtat->ItemIndex+(RadioGroupEtat->ItemIndex==2)),
           EtatFC=inactif,
           EtatCrt;
  bool FeuxFCDifferents=false;
  // On relève l'état des feux de circulation liés

  for(int i=0; i<NbFeux; i++)
   {
    if (!EtatFC)
     EtatFC=cv->Feu[NumFeux[i]-1].Etat;
    else
     if ((EtatCrt=cv->Feu[NumFeux[i]-1].Etat))
      FeuxFCDifferents=FeuxFCDifferents||(EtatFC!=EtatCrt);
   }
  if (!EtatFP) // Si pas d'état, on propose l'état opposé des feux liés
   if (EtatFC&&(!FeuxFCDifferents))
    if (AfficheMessage("\
Vous n'avez pas affecté d'état au feu de piétons.\n\
Voulez-vous affecter le feu par rapport aux feux de circulation liés ?",
                       "Assistant feu de piétons",
                       tmQuestionNecessaire, 0, 2)==trOui)
     switch(EtatFC)
      {
       case orange:
       case vert:   RadioGroupEtat->ItemIndex=2;
                    break;
       case rouge:  RadioGroupEtat->ItemIndex=1;
                    break;
       default:     break; // v5.4 (évite avertissement)
	  }
    else
     return;
   else
    return;
  else
   if (FeuxFCDifferents||(EtatFC!=4-EtatFP))
    if (AfficheMessage("\
Les feux de circulation liés n'ont pas tous l'état opposé à celui du feu de piéton.\n\
Voulez-vous affecter l'état opposé à tous les feux de circulation liés ?",
                       "Assistant feu de piétons",
                       tmQuestionNecessaire, 0, 2)==trOui)
     {
      for(int i=0; i<NbFeux; i++)
       {
        switch(EtatFP)
         {
          case vert:   cv->Feu[NumFeux[i]-1].Etat=rouge;
                       break;
          case rouge:  cv->Feu[NumFeux[i]-1].Etat=vert;
					   break;
          default:     break; // v5.4 (évite avertissement)
         }
        frmSimulation->RedessineCase(cv->Feu[NumFeux[i]-1].x, cv->Feu[NumFeux[i]-1].y, true);
       }
      return;
     }
    else
     return;
   else
    return;
 }
//---------------------------------------------------------------------------
bool TfrmDefFeuxPietons::AppliqueChoix()
 {
  int i;
  AnalyseEtatFeux(); // Assistant pour les états de feux (v5.2.1)
  frmSimulation->SimulationModifiee=true;
  f->Etat=(etat_feu)(RadioGroupEtat->ItemIndex+(RadioGroupEtat->ItemIndex==2));
  f->TypeDmdVert=(dmd_vert)RadioGroupTypeDmdVert->ItemIndex;
  f->TypePsgRouge=(psg_rouge)RadioGroupTypePsgRouge->ItemIndex;
  f->DureeVert=CSpinEditDureeVert->Value;
  f->DureeRouge=CSpinEditDureeRouge->Value;

  // Copie des listes de feux
  f->NbFeux=0;
  if (f->NumFeux) { delete[] f->NumFeux; f->NumFeux=NULL; }
  if ((NbFeux)&&(!(f->NumFeux=new int[NbFeux])))
   {
    AfficheMessage(asMemInsuff,asErrInattendue, tmErreur, 0, 0);
    return false;
   }
  f->NbFeux=NbFeux;
  for(i=0; i<NbFeux; i++) f->NumFeux[i]=NumFeux[i];
  ButtonAppliquer->Enabled=false;
  RafraichitFeuxPietons();
  return true;
 }
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::RafraichitInfosFeux()
 {
  ButtonAppliquer->Enabled=true;
  LabelNbFeux->Caption=IntToStr(NbFeux);
 }
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::FormShow(TObject *Sender)
{
 if (!RafraichitInfos())
  Hide();
 else
  RafraichitFeuxPietons();
}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::FormHide(TObject *Sender)
{
 int i;
 frmSimulation->ActionPrioritesPietons->Enabled=true;
 SpeedButtonSelFeux->Down=false;
 if (frmSimulation->SelectionFeuxPtn)
  for(i=0; i<NbFeux; i++)
   frmSimulation->RedessineCase(cv->Feu[NumFeux[i]-1].x, cv->Feu[NumFeux[i]-1].y, true);
 frmSimulation->SelectionFeuxPtn=false;
 RafraichitFeuxPietons();
 if (frmDefVoiePietons&&frmDefVoiePietons->Visible)
  frmDefVoiePietons->RafraichitInfos();
 frmSimulation->DrawGridSimul->Col=x;
 frmSimulation->DrawGridSimul->Row=y;
 frmSimulation->DrawGridSimulClick(frmSimulation);
}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::ButtonOkClick(TObject *Sender)
{
 AppliqueChoix();
 Hide();
}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::ButtonAnnulerClick(TObject *Sender)
{
 ButtonAppliquer->Enabled=false;
 Hide();
}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::ButtonAppliquerClick(TObject *Sender)
{
 if (!AppliqueChoix()) Hide();
}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::RadioGroupEtatClick(TObject *Sender)
{
 if ((etat_feu)(RadioGroupEtat->ItemIndex+(RadioGroupEtat->ItemIndex==2))!=f->Etat)
  ButtonAppliquer->Enabled=true;
}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::RadioGroupTypeDmdVertClick(TObject *Sender)
{
 if (RadioGroupTypeDmdVert->ItemIndex!=f->TypeDmdVert)
  ButtonAppliquer->Enabled=true;
}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::CSpinEditDureeVertChange(TObject *Sender)
{
 ButtonAppliquer->Enabled=true;
}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::SpeedButtonSelFeuxClick(TObject *Sender)
{
 int i;
 frmSimulation->SelectionFeuxPtn=SpeedButtonSelFeux->Down;
 for(i=0; i<NbFeux; i++) frmSimulation->RedessineCase(cv->Feu[NumFeux[i]-1].x, cv->Feu[NumFeux[i]-1].y, true);
 frmSimulation->RedessineCase(frmSimulation->DrawGridSimul->Col, frmSimulation->DrawGridSimul->Row, true);
 frmSimulation->ActionPriorites->Enabled=!frmSimulation->SelectionFeuxPtn;
}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::RadioGroupTypePsgRougeClick(TObject *Sender)
{
 if (RadioGroupTypePsgRouge->ItemIndex!=f->TypePsgRouge)
  ButtonAppliquer->Enabled=true;
}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::ButtonAideClick(TObject *Sender)
{ // v3.0
 AfficheAideContextuelle(this); // v3.8
}
//---------------------------------------------------------------------------
void TfrmDefFeuxPietons::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112)
   AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmDefFeuxPietons::Gestionnaire);
void TfrmDefFeuxPietons::ChargeFiche()
 {
  Ui::frmDefFeuxPietons().setupUi(this);
  LieurComposants Lieur(this, "f_deffp", Composants);
  COMPOSANTS_frmDefFeuxPietons(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmDefFeuxPietons(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------
