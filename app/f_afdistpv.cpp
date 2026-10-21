//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "base.h"
#include "simul.h"
#include "f_simul.h"
#include "f_afdistpv.h"
//---------------------------------------------------------------------------
#include "ui_f_afdistpv.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_afdistpv_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmAffDistPlacesVehlib *frmAffDistPlacesVehlib;
//---------------------------------------------------------------------------
const AnsiString asDir1[NBDIR+1]={"", "   ­", "   ®", "   ¯", "   ¬"};
//---------------------------------------------------------------------------
TfrmAffDistPlacesVehlib::TfrmAffDistPlacesVehlib(TComponent* Owner) : TForm(Owner) { ChargeFiche(); } // portage : chargement du .dfm, puis OnCreate
//---------------------------------------------------------------------------
void TfrmAffDistPlacesVehlib::EffaceInfos()
 {
  int i,
      NbCol=StringGridDistances->ColCount,
      NbLig=StringGridDistances->RowCount,
      NbCases=NbCol*NbLig;
  for(i=0; i<NbCases; i++) StringGridDistances->Cells[i%NbCol][i/NbCol]="";
 }
//---------------------------------------------------------------------------
void TfrmAffDistPlacesVehlib::RafraichitInfos()
 {
  int i, NbCases, NbX, de,
      x=frmSimulation->DrawGridSimul->Col,
      y=frmSimulation->DrawGridSimul->Row;
  voie *v=&(cv->v[x][y]);
  EffaceInfos();
  if (!v->DistPlaceVehlib) return;
  NbCases=(NbX=1+NbDir[v->DirPoss])*(1+cv->NbPlacesVehlib);
  Caption="Places vehlib "+Format(asFormatCoord, ARRAYOFCONST((x, y)));
  for(i=0; i<NbCases; i++)
   {
    if (i)
     {
      if (!(i%NbX)) StringGridDistances->Cells[i%NbX][i/NbX]=Format("%5d", ARRAYOFCONST((i/NbX)));
      if (i&&(i<NbX)) StringGridDistances->Cells[i%NbX][i/NbX]=asDir1[Dirdp[i%NbX-1][v->DirPoss]];
      if ((i%NbX)&&(i>=NbX)) // v5.4 : conversion BCB6>BCBX (AnsiString>String)
	   StringGridDistances->Cells[i%NbX][i/NbX]=((de=v->DistPlaceVehlib[i/NbX-1][i%NbX-1])==DIST_INDEFINIE)?String("     "):Format("%5d", ARRAYOFCONST((de)));
     }
   }
 }
//---------------------------------------------------------------------------
void TfrmAffDistPlacesVehlib::FormShow(TObject *Sender)
{
 frmSimulation->ActionDistancesPlacesVehlib->Checked=true;
 StringGridDistances->RowCount=1+cv->NbPlacesVehlib;
 Height=(cv->NbPlacesVehlib>10)?283:(26+21*StringGridDistances->RowCount);
 RafraichitInfos();
}
//---------------------------------------------------------------------------
void TfrmAffDistPlacesVehlib::FormHide(TObject *Sender)
{
 frmSimulation->ActionDistancesPlacesVehlib->Checked=false;
}
//---------------------------------------------------------------------------
void TfrmAffDistPlacesVehlib::CheckBoxCouloirsBusClick(
      TObject *Sender)
{
 RafraichitInfos();
}
//---------------------------------------------------------------------------
void TfrmAffDistPlacesVehlib::FormKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112)
   AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmAffDistPlacesVehlib::Gestionnaire);
void TfrmAffDistPlacesVehlib::ChargeFiche()
 {
  Ui::frmAffDistPlacesVehlib().setupUi(this);
  LieurComposants Lieur(this, "f_afdistpv", Composants);
  COMPOSANTS_frmAffDistPlacesVehlib(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmAffDistPlacesVehlib(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------
