//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "base.h"
#include "simul.h"
#include "f_afdistfp.h"
#include "f_simul.h"
//---------------------------------------------------------------------------
#include "ui_f_afdistfp.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_afdistfp_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmAffDistFilesPark *frmAffDistFilesPark;
//---------------------------------------------------------------------------
const AnsiString asDir1[NBDIR+1]={"", "   ­", "   ®", "   ¯", "   ¬"};
//---------------------------------------------------------------------------
TfrmAffDistFilesPark::TfrmAffDistFilesPark(TComponent* Owner) : TForm(Owner) { ChargeFiche(); } // portage : chargement du .dfm, puis OnCreate
//---------------------------------------------------------------------------
void TfrmAffDistFilesPark::EffaceInfos()
 {
  int i,
      NbCol=StringGridDistances->ColCount,
      NbLig=StringGridDistances->RowCount,
      NbCases=NbCol*NbLig;
  for(i=0; i<NbCases; i++) StringGridDistances->Cells[i%NbCol][i/NbCol]="";
 }
//---------------------------------------------------------------------------
void TfrmAffDistFilesPark::RafraichitInfos()
 {
  int i, NbCases, NbX, de,
      x=frmSimulation->DrawGridSimul->Col,
      y=frmSimulation->DrawGridSimul->Row;
  voie *v=&(cv->v[x][y]);
  EffaceInfos();
  if (!v->DistFilePark) return;
  NbCases=(NbX=1+NbDir[v->DirPoss])*(1+cv->NbFilesPark);
  Caption="Files parking "+Format(asFormatCoord, ARRAYOFCONST((x, y)));
  for(i=0; i<NbCases; i++)
   {
    if (i)
     {
      if (!(i%NbX)) StringGridDistances->Cells[i%NbX][i/NbX]=Format("%5d", ARRAYOFCONST((i/NbX)));
	  if (i&&(i<NbX)) StringGridDistances->Cells[i%NbX][i/NbX]=asDir1[Dirdp[i%NbX-1][v->DirPoss]];
	  if ((i%NbX)&&(i>=NbX)) // v5.4 : conversion BCB6>BCBX (AnsiString>String)
	   StringGridDistances->Cells[i%NbX][i/NbX]=((de=v->DistFilePark[i/NbX-1][i%NbX-1][CheckBoxCouloirsBus->Checked])==DIST_INDEFINIE)?String("     "):Format("%5d", ARRAYOFCONST((de)));
     }
   }
 }
//---------------------------------------------------------------------------
void TfrmAffDistFilesPark::FormShow(TObject *Sender)
{
 frmSimulation->ActionDistancesFilesPark->Checked=true;
 StringGridDistances->RowCount=1+cv->NbFilesPark;
 Height=(cv->NbFilesPark>10)?300:(43+21*StringGridDistances->RowCount); // v6.0.2 : avec la case « Avec couloirs de bus », comme f_afdistp (grille plus courte de 17 pixels)
 RafraichitInfos();
}
//---------------------------------------------------------------------------
void TfrmAffDistFilesPark::FormHide(TObject *Sender)
{
 frmSimulation->ActionDistancesFilesPark->Checked=false;
}
//---------------------------------------------------------------------------
void TfrmAffDistFilesPark::CheckBoxCouloirsBusClick(
      TObject *Sender)
{
 RafraichitInfos();
}
//---------------------------------------------------------------------------

void TfrmAffDistFilesPark::FormKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112)
   AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmAffDistFilesPark::Gestionnaire);
void TfrmAffDistFilesPark::ChargeFiche()
 {
  Ui::frmAffDistFilesPark().setupUi(this);
  LieurComposants Lieur(this, "f_afdistfp", Composants);
  COMPOSANTS_frmAffDistFilesPark(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmAffDistFilesPark(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------
