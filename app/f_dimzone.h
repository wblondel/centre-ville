//---------------------------------------------------------------------------
#ifndef f_dimzoneH
#define f_dimzoneH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//#include <Forms.hpp> // portage
//#include "CSPIN.h" // portage
//---------------------------------------------------------------------------
#include "base.h"
//---------------------------------------------------------------------------
class TfrmDimensionsZone : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
  TCSpinEdit *CSpinEditNbX;
  TCSpinEdit *CSpinEditNbY;
  TLabel *Label1;
  TLabel *Label2;
  TButton *ButtonCreer;
  TButton *ButtonAnnuler;
  TButton *ButtonAide;
  TCheckBox *CheckBoxAfficherParametres;
  TCheckBox *CheckBoxGenererReseau;
  void FormShow(TObject *Sender);
  void ButtonCreerClick(TObject *Sender);
  void ButtonAideClick(TObject *Sender);
  void FormKeyPress(TObject *Sender, char &Key);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
public:		// Déclarations de l'utilisateur
  int NbX, NbY;
  TfrmDimensionsZone(TComponent* Owner);
  void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmDimensionsZone *frmDimensionsZone;
extern bool DefinitDimensionsZone(int &NbX, int &NbY, bool &GenereReseau);
//---------------------------------------------------------------------------
#endif
