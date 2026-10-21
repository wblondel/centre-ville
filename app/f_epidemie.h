//---------------------------------------------------------------------------

#ifndef f_epidemieH
#define f_epidemieH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <System.Classes.hpp> // portage
//#include <Vcl.Controls.hpp> // portage
//#include <Vcl.StdCtrls.hpp> // portage
//#include <Vcl.Forms.hpp> // portage
//#include "cspin.h" // portage
//---------------------------------------------------------------------------
class TfrmEpidemie : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
	TGroupBox *GroupBox;
	TCSpinEdit *CSpinEditInfectiosite;
	TLabel *Label1;
	TCSpinEdit *CSpinEditPatientZero;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TCSpinEdit *CSpinEditChargeViraleFatale;
	TLabel *Label5;
	TButton *ButtonOk;
	TButton *ButtonAnnuler;
	TButton *ButtonAide;
	TLabel *Label6;
	TCheckBox *CheckBoxActiver;
	TLabel *Label7;
	TLabel *Label8;
	TCheckBox *CheckBoxReinfections;
	void ButtonOkClick(TObject *Sender);
	void ButtonAideClick(TObject *Sender);
	void FormShow(TObject *Sender);
private:	// Déclarations utilisateur
public:		// Déclarations utilisateur
	TfrmEpidemie(TComponent* Owner);
	void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmEpidemie *frmEpidemie;
//---------------------------------------------------------------------------
#endif
