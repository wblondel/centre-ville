//---------------------------------------------------------------------------
#ifndef f_infptnH
#define f_infptnH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "simul.h"
//---------------------------------------------------------------------------
class TfrmInfosPieton : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TButton *ButtonOk;
    TLabel *Label0;
    TLabel *LabelCoordonnees;
    TRadioButton *RadioButtonAucunPieton;
    TGroupBox *GroupBoxPieton;
    TRadioButton *RadioButtonPietonNumero;
    TLabel *Label3;
    TLabel *LabelSituation;
    TLabel *Label4;
    TLabel *LabelParcours;
    TLabel *Label1;
    TLabel *LabelDirection;
    TLabel *Label2;
    TLabel *LabelPosition;
    TButton *ButtonAide;
    TLabel *Label5;
        TLabel *LabelBusTramTaxiVehlib;
    TLabel *Label6;
        TLabel *LabelArretBusTramFileTaxiVehlib;
	TGroupBox *GroupBoxEpidemie;
	TCheckBox *CheckBoxEpidemie;
	TLabel *Label7;
	TLabel *LabelInfecte;
	TLabel *Label8;
	TLabel *LabelRangs;
	TLabel *Label9;
	TLabel *LabelTours;
	TLabel *Label10;
	TLabel *LabelInfections;
	TLabel *Label11;
	TLabel *LabelPeriodeInfectiosite;
	TLabel *Label13;
	TLabel *LabelChargeVirale;
	TLabel *Label12;
	TLabel *LabelEtatFinal;
    void FormShow(TObject *Sender);
    void FormHide(TObject *Sender);
    void ButtonOkClick(TObject *Sender);
    void ButtonAideClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    
private:	// Déclarations de l'utilisateur
    voie *v;
    pieton *p;
    int x, y, NumPieton;
    void RendreVisibleInfos(bool Visible);
public:		// Déclarations de l'utilisateur
    TfrmInfosPieton(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void RafraichitInfos();
};
//---------------------------------------------------------------------------
extern TfrmInfosPieton *frmInfosPieton;
//---------------------------------------------------------------------------
#endif
  