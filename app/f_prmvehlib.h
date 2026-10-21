//---------------------------------------------------------------------------
#ifndef f_prmvehlibH
#define f_prmvehlibH
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
class TfrmParamVehlib : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
        TLabel *Label21;
        TCSpinEdit *CSpinEditNbMaxPersonnesVehlib;
        TLabel *Label22;
        TGroupBox *GroupBox;
        TLabel *LabelNbCasesMaxPsgPtn;
        TLabel *Label23;
        TLabel *LabelProbaVehlib;
        TLabel *Label25;
        TCSpinEdit *CSpinEditNbCasesMaxPlaceVehlib;
        TCSpinEdit *CSpinEditProbaDirPlaceVehlib;
        TButton *ButtonOk;
        TButton *ButtonAnnuler;
        TButton *ButtonAppliquer;
        TButton *ButtonAide;
        TLabel *Label1;
        TCSpinEdit *CSpinEditTauxRemplissage;
        TLabel *Label2;
        TLabel *Label3;
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
public:	 // Déclarations de l'utilisateur
        TfrmParamVehlib(TComponent* Owner);
        void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmParamVehlib *frmParamVehlib;
//---------------------------------------------------------------------------
#endif
