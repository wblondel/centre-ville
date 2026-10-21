//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "simul.h"
#include "f_simul.h"
#include "f_prmvehlib.h"
//---------------------------------------------------------------------------
#include "ui_f_prmvehlib.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
//#pragma link "cspin" // portage : TCSpinEdit dans vcl/controles.h
#include "f_prmvehlib_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmParamVehlib *frmParamVehlib;
//---------------------------------------------------------------------------
TfrmParamVehlib::TfrmParamVehlib(TComponent* Owner)
        : TForm(Owner)
{ ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
}
//---------------------------------------------------------------------------
void TfrmParamVehlib::ButtonOkClick(TObject *Sender)
{
 AppliqueChoix();
 ModalResult=mrOk;
}
//---------------------------------------------------------------------------
void TfrmParamVehlib::ButtonAppliquerClick(TObject *Sender)
{
 AppliqueChoix();
}
//---------------------------------------------------------------------------
void TfrmParamVehlib::ButtonAideClick(TObject *Sender)
{
 AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
void TfrmParamVehlib::FormShow(TObject *Sender)
{
 InitialisationEnCours=true;
 CSpinEditNbMaxPersonnesVehlib->Value=cv->NbMaxPersonnesVehlib;
 CSpinEditNbCasesMaxPlaceVehlib->Value=cv->NbMaxCasesDetectFileVehlib;
 CSpinEditProbaDirPlaceVehlib->Value=cv->ProbaDirFileVehlib;
 CSpinEditTauxRemplissage->Value=cv->TauxRemplissageVehlib;
 InitialisationEnCours=false;
}
//---------------------------------------------------------------------------
void TfrmParamVehlib::AppliqueChoix()
 {
  cv->NbMaxPersonnesVehlib=CSpinEditNbMaxPersonnesVehlib->Value;
  cv->NbMaxCasesDetectFileVehlib=CSpinEditNbCasesMaxPlaceVehlib->Value;
  cv->ProbaDirFileVehlib=CSpinEditProbaDirPlaceVehlib->Value;
  cv->TauxRemplissageVehlib=CSpinEditTauxRemplissage->Value;
  frmSimulation->SimulationModifiee=true;
  ButtonAppliquer->Enabled=false;
 }
//---------------------------------------------------------------------------
void TfrmParamVehlib::CSpinEditParametresChange(
      TObject *Sender)
{
 if (!InitialisationEnCours) ButtonAppliquer->Enabled=true;
}
//---------------------------------------------------------------------------
void TfrmParamVehlib::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112)
   AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmParamVehlib::Gestionnaire);
void TfrmParamVehlib::ChargeFiche()
 {
  Ui::frmParamVehlib().setupUi(this);
  LieurComposants Lieur(this, "f_prmvehlib", Composants);
  COMPOSANTS_frmParamVehlib(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmParamVehlib(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------
