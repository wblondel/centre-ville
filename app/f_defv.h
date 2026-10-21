//---------------------------------------------------------------------------
#ifndef f_defvH
#define f_defvH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Buttons.hpp> // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <ExtCtrls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "simul.h"
//---------------------------------------------------------------------------
class TfrmDefVoie : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TLabel *Label1;
    TLabel *LabelCoordonnees;
    TRadioGroup *RadioGroupPrioriteNord;
    TRadioGroup *RadioGroupPrioriteOuest;
    TRadioGroup *RadioGroupPrioriteEst;
    TRadioGroup *RadioGroupPrioriteSud;
    TButton *ButtonOk;
    TButton *ButtonAnnuler;
    TBitBtn *BitBtnDefFeuN;
    TBitBtn *BitBtnDefFeuO;
    TBitBtn *BitBtnDefFeuE;
    TBitBtn *BitBtnDefFeuS;
    TButton *ButtonAppliquer;
    TButton *ButtonAide;
    void FormShow(TObject *Sender);
    void ButtonOkClick(TObject *Sender);
    void ButtonAppliquerClick(TObject *Sender);
    void ButtonAnnulerClick(TObject *Sender);
    void FormHide(TObject *Sender);
    void ButtonAideClick(TObject *Sender);
    void BitBtnDefFeuClick(TObject *Sender);
    void FormCreate(TObject *Sender);
    void RadioGroupPrioriteClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
    voie *v;
    int x, y;
    TBitBtn *BitBtn[NBDIR]; // v5.2 
    void DefinitFeu(direction Dir);
public:		// Déclarations de l'utilisateur
    TfrmDefVoie(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void RafraichitInfos();
    void AppliqueChoix();
};
//---------------------------------------------------------------------------
extern TfrmDefVoie *frmDefVoie;
//---------------------------------------------------------------------------
#endif
