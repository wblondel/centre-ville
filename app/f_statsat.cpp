//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "simul.h"
#include "f_statsat.h"
#include "f_simul.h"
//---------------------------------------------------------------------------
#include "ui_f_statsat.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
//#pragma link "cspin" // portage
#include "f_statsat_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
#define ATTENTE 0
#define TRAFIC  1
//---------------------------------------------------------------------------
TfrmStatsAttenteTrafic *frmStatsAttenteTrafic;
//---------------------------------------------------------------------------
TfrmStatsAttenteTrafic::TfrmStatsAttenteTrafic(TComponent* Owner)
    : TForm(Owner)
{ m[0]=0; m[1]=0; ChargeFiche(); // portage : membres à zéro (TObject::NewInstance), chargement du .dfm, puis OnCreate
 // v5.4 on charge les glyphes manuellement pour prendre en compte le HDPI
 for(int i=0; i<9; i++)
  if (i!=4) // v5.4 : plus de zoom !
   frmSimulation->AffecteGlyphe(GLYPHE_DIR+i, ((TBitBtn*)(FindComponent((UnicodeString)("BitBtn"+IntToStr(1+i%3)+IntToStr(1+i/3)))))->Glyph);
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::PaintBoxPaint(
      TObject *Sender)
{
 for(int i=0; i<128; i++)
  {
   PaintBox->Canvas->Pen->Color=(TColor)RGB((i<64)?4*i:252,(i>=64)?252-4*(i-64):252,0);
   PaintBox->Canvas->MoveTo(i, 0);
   PaintBox->Canvas->LineTo(i, PaintBox->Height-1);
  }
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::FormShow(TObject *Sender)
{
 if (cv->TourCrt)
  Caption="Statistiques de fluidité et de trafic au tour n°"+IntToStr(cv->TourCrt);
 CSpinEditParkingDepart->MAXVALUE=cv->NbParkings;
 CSpinEditParkingArrivee->MAXVALUE=cv->NbParkings;
 RecalculeEtVisualise();
}
//---------------------------------------------------------------------------
TColor TfrmStatsAttenteTrafic::CouleurCase(int x, int y)
 {
  int s,
      ed=CheckBoxParkingDepart->Checked?CSpinEditParkingDepart->Value:0,
      ea=CheckBoxParkingArrivee->Checked?CSpinEditParkingArrivee->Value:0,
      ts=RadioGroupType->ItemIndex;
  if (m[ts]&&cv->v[x][y].Trafic&&cv->v[x][y].Attente) // v3.0
   {
    if (ts)
     s=(127*cv->v[x][y].Trafic[ed][ea])/m[ts];
    else
     s=(127*cv->v[x][y].Attente[ed][ea])/m[ts];
    return (TColor)RGB((s<64)?4*s:252,(s>=64)?252-4*(s-64):252,0);
   }
  else
   return (TColor)0;
 }
//---------------------------------------------------------------------------
AnsiString TfrmStatsAttenteTrafic::asMsgDetailCase(int x, int y)
 {  // v3.0
  AnsiString asMsg;
  int ed=CheckBoxParkingDepart->Checked?CSpinEditParkingDepart->Value:0,
      ea=CheckBoxParkingArrivee->Checked?CSpinEditParkingArrivee->Value:0,
      ts=RadioGroupType->ItemIndex;
  if (m[ts]&&cv->v[x][y].Trafic&&cv->v[x][y].Attente)
   {
	if (ts)
	 {
	  asMsg=Format( "Trafic en (%d, %d) au tour %d : %d véhicules (Max. %d) soit %.3f%%",
					ARRAYOFCONST(( x, y, (int)cv->TourCrt,
								   cv->v[x][y].Trafic[ed][ea], m[ts],
								   (long double)((100.0*cv->v[x][y].Trafic[ed][ea])/m[ts]))));
	 }
	else
	 {
	  asMsg=Format( "Attente en (%d, %d) au tour %d : %d tours (Max. %d) soit %.3f%%",
					ARRAYOFCONST(( x, y, (int)cv->TourCrt,
								   cv->v[x][y].Attente[ed][ea], m[ts],
								   (long double)((100.0*cv->v[x][y].Attente[ed][ea])/m[ts]))));
	 }
   }
  return asMsg;
 }
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::RecalculeEtVisualise()
{
 int x, y, n=0, t[2]={0,0},
     ed=CheckBoxParkingDepart->Checked?CSpinEditParkingDepart->Value:0,
     ea=CheckBoxParkingArrivee->Checked?CSpinEditParkingArrivee->Value:0;
 if ((ed<0)||(ed>cv->NbParkings)||(ea<0)||(ea>cv->NbParkings))
  {
   AfficheMessage("Le n° d'étape est incorrect !", "Valeur incorrecte", tmAvertissement, 0, 0);
   return;
  }
 m[0]=0; m[1]=0;
 for(x=0; x<cv->NbX; x++)
  for(y=0; y<cv->NbY; y++)
   if ((cv->v[x][y].DirPoss>aucune)&&(!cv->v[x][y].NumParking))
    {
     n++;
     if (cv->v[x][y].Attente&&cv->v[x][y].Trafic)
      {
       if (cv->v[x][y].Attente[ed][ea]>m[ATTENTE]) m[ATTENTE]=cv->v[x][y].Attente[ed][ea];
       t[ATTENTE]+=cv->v[x][y].Attente[ed][ea];
       if (cv->v[x][y].Trafic[ed][ea]>m[TRAFIC]) m[TRAFIC]=cv->v[x][y].Trafic[ed][ea];
       t[TRAFIC]+=cv->v[x][y].Trafic[ed][ea];
      }
    }
 LabelMax->Caption=IntToStr(m[RadioGroupType->ItemIndex]);
 LabelAttenteCumulee->Caption=IntToStr(t[ATTENTE]);
 if (n) // v1.8.4 : FormatFloat à la place de Format
  LabelAttenteMoyenne->Caption=FormatFloat("0.0", 1.0*t[ATTENTE]/n); // v3.0 (ajout). v5.4.1 : FormatFloat n'utilise pas le format "%.1f" mais "#.0"
 else
  LabelAttenteMoyenne->Caption="N/D";
 LabelAttenteMaximale->Caption=IntToStr(m[ATTENTE]);
 LabelTraficCumule->Caption=IntToStr(t[TRAFIC]);
 if (n) // v1.8.4 : FormatFloat à la place de Format
  LabelTraficMoyen->Caption=FormatFloat("0.0", 1.0*t[TRAFIC]/n); // v5.4.1 : FormatFloat n'utilise pas le format "%.1f" mais "#.0"
 else
  LabelTraficMoyen->Caption="N/D";
 LabelTraficMaximal->Caption=IntToStr(m[TRAFIC]);
 frmSimulation->DrawGridSimul->Repaint();
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::ParametresParkingsClick(TObject *Sender)
{
 switch(TagDe((TComponent*)Sender)) // Comportements propres à certains objets // portage : TComponent::Tag -> TagDe
  {
   case 1: CSpinEditParkingDepart->Enabled=CheckBoxParkingDepart->Checked;
           if (CSpinEditParkingDepart->Enabled) CSpinEditParkingDepart->SetFocus();
           break;
   case 2: CSpinEditParkingArrivee->Enabled=CheckBoxParkingArrivee->Checked;
           if (CSpinEditParkingArrivee->Enabled) CSpinEditParkingArrivee->SetFocus();
           break;
  }
 RecalculeEtVisualise();
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::RadioGroupTypeClick(
      TObject *Sender)
{
 LabelMax->Caption=IntToStr(m[RadioGroupType->ItemIndex]);
 frmSimulation->DrawGridSimul->Repaint();
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::BitBtnDirClick(
      TObject *Sender)
{
 int n=StrToInt(NomDe((TComponent*)Sender).SubString(7, NomDe((TComponent*)Sender).Length()-6)), // portage : TComponent::Name -> NomDe
     dx=n/10-2, dy=n%10-2;
 if (CheckBoxCoins->Checked)
  {
   switch(dx)
    {
     case -1: frmSimulation->DrawGridSimul->Col=0; break;
     case +1: frmSimulation->DrawGridSimul->Col=frmSimulation->DrawGridSimul->ColCount-1; break;
    }
   switch(dy)
    {
     case -1: frmSimulation->DrawGridSimul->Row=0; break;
     case +1: frmSimulation->DrawGridSimul->Row=frmSimulation->DrawGridSimul->RowCount-1; break;
    }
  }
 else
  {
   if ((frmSimulation->DrawGridSimul->Col+dx>=0)&&
       (frmSimulation->DrawGridSimul->Col+dx<frmSimulation->DrawGridSimul->ColCount))
    frmSimulation->DrawGridSimul->Col+=dx;
   if ((frmSimulation->DrawGridSimul->Row+dy>=0)&&
       (frmSimulation->DrawGridSimul->Row+dy<frmSimulation->DrawGridSimul->RowCount))
    frmSimulation->DrawGridSimul->Row+=dy;
  }
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::RubriqueClick(
      TObject *Sender)
{
 TBitBtn *BitBtn=(TBitBtn*)FindComponent("BitBtn"+IntToStr(TagDe((TComponent*)Sender))); // portage : TComponent::Tag -> TagDe
 if (BitBtn) BitBtn->Click(); // portage : « Zoom » (Tag 22) : pas de BitBtn22 depuis la v5.4, violation d'accès dans l'original
}
//---------------------------------------------------------------------------

void TfrmStatsAttenteTrafic::FormKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112)
   AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmStatsAttenteTrafic::Gestionnaire);
void TfrmStatsAttenteTrafic::ChargeFiche()
 {
  Ui::frmStatsAttenteTrafic().setupUi(this);
  LieurComposants Lieur(this, "f_statsat", Composants);
  COMPOSANTS_frmStatsAttenteTrafic(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmStatsAttenteTrafic(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------
