//---------------------------------------------------------------------------
#ifndef f_prmsimH
#define f_prmsimH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include "CSPIN.h" // portage
//#include <Buttons.hpp> // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <ExtCtrls.hpp> // portage
//#include <StdCtrls.hpp> // portage
#include "base.h"
//---------------------------------------------------------------------------
class TfrmParamSimul : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TButton *ButtonOk;
    TButton *ButtonAnnuler;
    TGroupBox *GroupBoxNbElements;
    TLabel *LabelNbVehicules;
	TCSpinEdit *CSpinEditNbVehicules;
	TGroupBox *GroupBoxFonctionnement;
	TCSpinEdit *CSpinEditAttenteAvChx;
	TLabel *LabelTours;
	TLabel *LabelAttenteAvChx;
	TButton *ButtonAppliquer;
	TLabel *LabelNbToursParSeconde;
	TCSpinEdit *CSpinEditNbToursParSeconde;
	TLabel *LabelDureeOrange;
	TCSpinEdit *CSpinEditDureeOrange;
	TLabel *LabelDureeRouge;
	TCSpinEdit *CSpinEditDureeRouge;
	TLabel *LabelDureeVert;
	TCSpinEdit *CSpinEditDureeVert;
	TBitBtn *BitBtnAppliqueDureeFeuVert;
	TBitBtn *BitBtnAppliqueDureeFeuRouge;
	TRadioGroup *RadioGroupVitesses;
	TLabel *LabelNbPietons;
	TCSpinEdit *CSpinEditNbPietons;
	TLabel *Label1;
	TGroupBox *GroupBoxPietons;
	TLabel *LabelNbCasesMaxPsgPtn;
	TCSpinEdit *CSpinEditNbCasesMaxPsgPtn;
	TLabel *Label4;
	TLabel *LabelProbaPsgPt;
	TCSpinEdit *CSpinEditProbaPsgPt;
	TLabel *Label6;
	TLabel *Label7;
	TLabel *LabelNbArretsBus;
	TBitBtn *BitBtnDefLignesBus;
	TButton *ButtonAide;
	TLabel *LabelNbArretsTram;
	TBitBtn *BitBtnDefLignesTram;
	TLabel *LabelNbPlacesTaxi;
	TBitBtn *BitBtnDefTaxis;
	TLabel *Label2;
	TBitBtn *BitBtnDefVehlibs;
	void CSpinEditNbVehiculesChange(TObject *Sender);
	void ButtonOkClick(TObject *Sender);

	void ButtonAppliquerClick(TObject *Sender);
	void FormShow(TObject *Sender);

	void BitBtnAppliqueDureeFeuVertClick(TObject *Sender);
	void BitBtnAppliqueDureeFeuRougeClick(TObject *Sender);
	void CSpinEditDureeVertChange(TObject *Sender);
	void CSpinEditDureeRougeChange(TObject *Sender);
    void CSpinEditNbPietonsChange(TObject *Sender);
    void Change(TObject *Sender);
    void BitBtnDefLignesBusClick(TObject *Sender);
    
    void ButtonAideClick(TObject *Sender);
    void BitBtnDefLignesTramClick(TObject *Sender);
    void BitBtnDefTaxisClick(TObject *Sender);
    void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void BitBtnDefVehlibsClick(TObject *Sender);
private:	// Déclarations de l'utilisateur
    bool NbVehiculesChange,
         NbPietonsChange, // v2.0
         NbFeuxChange,
         NbParkingsChange,
         NbArretsBusChange, // v3.0
         NbArretsTramChange, // v3.5
         NbPlacesTaxiChange, // v3.6
         NbPlacesParkChange, // v5.0
		 RetourFicheFille, // v6.0.2 : prochain FormShow au retour d'une fiche fille
		 Initialisation;
	TBITMAP *Bitmap; // v5.4
	bool AppliqueChoix();
    void CalculeChoixPossiblesNbVehicules(); // v5.0
public:		// Déclarations de l'utilisateur
    TfrmParamSimul(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmParamSimul *frmParamSimul;
extern void Parametres();
//---------------------------------------------------------------------------
#endif
