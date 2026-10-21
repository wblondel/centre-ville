//---------------------------------------------------------------------------
#ifndef f_defnH
#define f_defnH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//#include <Forms.hpp> // portage
//#include <ExtCtrls.hpp> // portage
//---------------------------------------------------------------------------
class TfrmDefNature : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
  TRadioGroup *RadioGroupNature;
  TCheckBox *CheckBoxEcraser;
  TButton *ButtonAppliquer;
  TButton *ButtonFermer;
  TButton *ButtonAide;
  TLabel *LabelRq;
  TCheckBox *CheckBoxPelouse;
  void RadioGroupNatureClick(TObject *Sender);
  void ButtonAppliquerClick(TObject *Sender);
  void ButtonAideClick(TObject *Sender);
  void FormShow(TObject *Sender);
  void FormHide(TObject *Sender);
  void ButtonFermerClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
public:		// Déclarations de l'utilisateur
  TfrmDefNature(TComponent* Owner);
  void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmDefNature *frmDefNature;
//---------------------------------------------------------------------------
#endif
