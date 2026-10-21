//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "base.h"
#include "simul.h"
#include "f_simul.h"
#include "f_epidemie.h"
//---------------------------------------------------------------------------
#include "ui_f_epidemie.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
//#pragma link "cspin" // portage : TCSpinEdit dans vcl/controles.h
#include "f_epidemie_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmEpidemie *frmEpidemie;
//---------------------------------------------------------------------------
TfrmEpidemie::TfrmEpidemie(TComponent* Owner)
	: TForm(Owner)
{ ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
  // v5.4.2 : Tout est fait dans FormShow
}
//---------------------------------------------------------------------------
void TfrmEpidemie::ButtonOkClick(TObject *Sender)
{
 // On enregistre les paramètres
 cv->EpidemieInfectiosite = CSpinEditInfectiosite->Value;
 cv->EpidemieIterationPatientZero = CSpinEditPatientZero->Value;
 cv->EpidemieChargeFatale = CSpinEditChargeViraleFatale->Value;
 cv->EpidemieReinfections = CheckBoxReinfections->Checked; // v5.4.2
}
//---------------------------------------------------------------------------

void TfrmEpidemie::ButtonAideClick(TObject *Sender)
{
 AfficheAideContextuelle(this);
}
//---------------------------------------------------------------------------

void TfrmEpidemie::FormShow(TObject *Sender)
{
 // On charge les paramètres
 CSpinEditInfectiosite->Value = cv->EpidemieInfectiosite; // Nb tours (100 par défaut)
 CSpinEditPatientZero->Value = cv->EpidemieIterationPatientZero; // Nb piétons (100 par défaut)
 CSpinEditChargeViraleFatale->Value = cv->EpidemieChargeFatale; // en % piétons infectés au bout de la période d'infectiosité (5 par défaut)
 CheckBoxReinfections->Checked = cv->EpidemieReinfections; // v5.4.2 : Si vrai, alors même guéri, un piéton peut être réinfecté, si faux, il ne peut plus l'être (faux par défaut)

 // On rafraîchit les accès. v5.4.2 : on ne teste plus CheckBoxActiver->Enabled
 CheckBoxActiver->Checked = frmSimulation->ActionEpidemie->Checked;
 GroupBox->Enabled = CheckBoxActiver->Checked;
 CSpinEditInfectiosite->Enabled = CheckBoxActiver->Checked;
 CSpinEditPatientZero->Enabled = CheckBoxActiver->Checked;
 CSpinEditChargeViraleFatale->Enabled = CheckBoxActiver->Checked;
 CheckBoxReinfections->Enabled = CheckBoxActiver->Checked; // v5.4.2
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmEpidemie::Gestionnaire);
void TfrmEpidemie::ChargeFiche()
 {
  Ui::frmEpidemie().setupUi(this);
  LieurComposants Lieur(this, "f_epidemie", Composants);
  COMPOSANTS_frmEpidemie(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmEpidemie(CONNECTE_EVENEMENT)
  if (OnCreate) OnCreate(this);
 }
//---------------------------------------------------------------------------
