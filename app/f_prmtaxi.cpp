//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "simul.h"
#include "f_simul.h"
#include "f_prmtaxi.h"
//---------------------------------------------------------------------------
#include "ui_f_prmtaxi.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
//#pragma link "cspin" // portage : TCSpinEdit dans vcl/controles.h
#include "f_prmtaxi_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmParamTaxi *frmParamTaxi;
//---------------------------------------------------------------------------
TfrmParamTaxi::TfrmParamTaxi(TComponent* Owner)
        : TForm(Owner)
{ ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
}
//---------------------------------------------------------------------------
void TfrmParamTaxi::ButtonOkClick(TObject *Sender)
{
 AppliqueChoix();
 ModalResult=mrOk;
}
//---------------------------------------------------------------------------
void TfrmParamTaxi::ButtonAppliquerClick(TObject *Sender)
{
 AppliqueChoix();
}
//---------------------------------------------------------------------------
void TfrmParamTaxi::ButtonAideClick(TObject *Sender)
{
 AfficheAideContextuelle(this); // v3.8
}
//---------------------------------------------------------------------------
void TfrmParamTaxi::FormShow(TObject *Sender)
{
 InitialisationEnCours=true;
 CSpinEditNbMaxPersonnesTaxi->Value=cv->NbMaxPersonnesTaxi;
 CSpinEditNbCasesMaxPlaceTaxi->Value=cv->NbMaxCasesDetectFileTaxi;
 CSpinEditProbaDirPlaceTaxi->Value=cv->ProbaDirFileTaxi;
 CSpinEditNbMaxToursAttente->Value=cv->NbMaxToursAttenteTaxi;
 CSpinEditProbaArretDmd->Value=cv->ProbaArretTaxiDmd;
 InitialisationEnCours=false;
}
//---------------------------------------------------------------------------
void TfrmParamTaxi::AppliqueChoix()
 {
  cv->NbMaxPersonnesTaxi=CSpinEditNbMaxPersonnesTaxi->Value;
  cv->NbMaxCasesDetectFileTaxi=CSpinEditNbCasesMaxPlaceTaxi->Value;
  cv->ProbaDirFileTaxi=CSpinEditProbaDirPlaceTaxi->Value;
  cv->NbMaxToursAttenteTaxi=CSpinEditNbMaxToursAttente->Value;
  cv->ProbaArretTaxiDmd=CSpinEditProbaArretDmd->Value;
  frmSimulation->SimulationModifiee=true;
  ButtonAppliquer->Enabled=false;
 }
//---------------------------------------------------------------------------
void TfrmParamTaxi::CSpinEditParametresChange(
      TObject *Sender)
{
 if (!InitialisationEnCours) ButtonAppliquer->Enabled=true;
}
//---------------------------------------------------------------------------
void TfrmParamTaxi::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112)
   AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmParamTaxi::Gestionnaire);
void TfrmParamTaxi::ChargeFiche()
 {
  Ui::frmParamTaxi().setupUi(this);
  LieurComposants Lieur(this, "f_prmtaxi", Composants);
  COMPOSANTS_frmParamTaxi(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmParamTaxi(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------
