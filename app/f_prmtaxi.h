//---------------------------------------------------------------------------
#ifndef f_prmtaxiH
#define f_prmtaxiH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include "CSPIN.h" // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "base.h"
//---------------------------------------------------------------------------
class TfrmParamTaxi : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
        TGroupBox *GroupBox;
        TLabel *LabelNbCasesMaxPsgPtn;
        TLabel *Label23;
        TLabel *Label24;
        TLabel *Label17;
        TLabel *LabelProbaPsgPt;
        TLabel *Label25;
        TLabel *Label26;
        TLabel *Label27;
        TCSpinEdit *CSpinEditNbCasesMaxPlaceTaxi;
        TCSpinEdit *CSpinEditNbMaxToursAttente;
        TCSpinEdit *CSpinEditProbaDirPlaceTaxi;
        TCSpinEdit *CSpinEditProbaArretDmd;
        TButton *ButtonOk;
        TButton *ButtonAnnuler;
        TButton *ButtonAppliquer;
        TButton *ButtonAide;
        TLabel *Label21;
        TCSpinEdit *CSpinEditNbMaxPersonnesTaxi;
        TLabel *Label22;
        void ButtonOkClick(TObject *Sender);
        void ButtonAppliquerClick(TObject *Sender);
        void ButtonAideClick(TObject *Sender);
        void FormShow(TObject *Sender);
        void CSpinEditParametresChange(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private: // Déclarations de l'utilisateur
        bool InitialisationEnCours;
        void AppliqueChoix();
public:	// Déclarations de l'utilisateur
        TfrmParamTaxi(TComponent* Owner);
        void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmParamTaxi *frmParamTaxi;
//---------------------------------------------------------------------------
#endif
