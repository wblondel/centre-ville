//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "base.h"
#include "simul.h"
#include "f_afdistft.h"
#include "f_simul.h"
//---------------------------------------------------------------------------
#include "ui_f_afdistft.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_afdistft_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmAffDistFilesTaxi *frmAffDistFilesTaxi;
//---------------------------------------------------------------------------
const AnsiString asDir1[NBDIR+1]={"", "   ­", "   ®", "   ¯", "   ¬"};
//---------------------------------------------------------------------------
TfrmAffDistFilesTaxi::TfrmAffDistFilesTaxi(TComponent* Owner) : TForm(Owner) { ChargeFiche(); } // portage : chargement du .dfm, puis OnCreate
//---------------------------------------------------------------------------
void TfrmAffDistFilesTaxi::EffaceInfos()
 {
  int i,
      NbCol=StringGridDistances->ColCount,
      NbLig=StringGridDistances->RowCount,
      NbCases=NbCol*NbLig;
  for(i=0; i<NbCases; i++) StringGridDistances->Cells[i%NbCol][i/NbCol]="";
 }
//---------------------------------------------------------------------------
void TfrmAffDistFilesTaxi::RafraichitInfos()
 {
  int i, NbCases, NbX, de,
      x=frmSimulation->DrawGridSimul->Col,
      y=frmSimulation->DrawGridSimul->Row;
  voie *v=&(cv->v[x][y]);
  EffaceInfos();
  if (!v->DistFileTaxi) return;
  NbCases=(NbX=1+NbDir[v->DirPoss])*(1+cv->NbFilesTaxi);
  Caption="Files taxi "+Format(asFormatCoord, ARRAYOFCONST((x, y)));
  for(i=0; i<NbCases; i++)
   {
    if (i)
     {
      if (!(i%NbX)) StringGridDistances->Cells[i%NbX][i/NbX]=Format("%5d", ARRAYOFCONST((i/NbX)));
      if (i&&(i<NbX)) StringGridDistances->Cells[i%NbX][i/NbX]=asDir1[Dirdp[i%NbX-1][v->DirPoss]];
      if ((i%NbX)&&(i>=NbX)) // v5.4 : conversion BCB6>BCBX (AnsiString>String)
       StringGridDistances->Cells[i%NbX][i/NbX]=((de=v->DistFileTaxi[i/NbX-1][i%NbX-1])==DIST_INDEFINIE)?String("     "):Format("%5d", ARRAYOFCONST((de)));
     }
   }
 }
//---------------------------------------------------------------------------
void TfrmAffDistFilesTaxi::FormShow(TObject *Sender)
{
 frmSimulation->ActionDistancesFilesTaxi->Checked=true;
 StringGridDistances->RowCount=1+cv->NbFilesTaxi;
 StringGridDistances->FixedRows=1; // v6.0.2 : RowCount=1 (tableau vide) l'avait ramené à 0, pour toutes les cartes suivantes (borné à RowCount-1)
 Height=(cv->NbFilesTaxi>10)?283:(26+21*StringGridDistances->RowCount);
 RafraichitInfos();
}
//---------------------------------------------------------------------------
void TfrmAffDistFilesTaxi::FormHide(TObject *Sender)
{
 frmSimulation->ActionDistancesFilesTaxi->Checked=false;
}
//---------------------------------------------------------------------------

void TfrmAffDistFilesTaxi::FormKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112)
   AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmAffDistFilesTaxi::Gestionnaire);
void TfrmAffDistFilesTaxi::ChargeFiche()
 {
  Ui::frmAffDistFilesTaxi().setupUi(this);
  LieurComposants Lieur(this, "f_afdistft", Composants);
  COMPOSANTS_frmAffDistFilesTaxi(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmAffDistFilesTaxi(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------
