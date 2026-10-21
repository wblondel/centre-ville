//---------------------------------------------------------------------------
#ifndef f_deffpH
#define f_deffpH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include "CSPIN.h" // portage
//#include <Buttons.hpp> // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <ExtCtrls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "base.h"
#include "simul.h"
//---------------------------------------------------------------------------
class TfrmDefFeuxPietons : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TLabel *Label1;
    TLabel *LabelCoordonnees;
    TLabel *Label2;
    TLabel *LabelDirection;
    TLabel *Label3;
    TLabel *LabelNumero;
    TRadioGroup *RadioGroupEtat;
    TRadioGroup *RadioGroupTypeDmdVert;
    TGroupBox *GroupBoxDureeFeu;
    TCSpinEdit *CSpinEditDureeRouge;
    TLabel *Label5;
    TCSpinEdit *CSpinEditDureeVert;
    TLabel *Label4;
    TButton *ButtonOk;
    TButton *ButtonAnnuler;
    TButton *ButtonAppliquer;
    TRadioGroup *RadioGroupTypePsgRouge;
    TLabel *Label7;
    TLabel *LabelNbFeux;
    TSpeedButton *SpeedButtonSelFeux;
    TButton *ButtonAide;
    TLabel *Label6;
    TLabel *LabelCoordonneesFeuPietonsOppose;
    TLabel *Label8;
    TLabel *Label9;
    TLabel *LabelDureeTraversee;
    void FormShow(TObject *Sender);
    void ButtonOkClick(TObject *Sender);
    
    void RadioGroupEtatClick(TObject *Sender);
    void RadioGroupTypeDmdVertClick(TObject *Sender);
    
    void CSpinEditDureeVertChange(TObject *Sender);
    void ButtonAppliquerClick(TObject *Sender);
    void ButtonAnnulerClick(TObject *Sender);
    void FormHide(TObject *Sender);
    void SpeedButtonSelFeuxClick(TObject *Sender);
    void RadioGroupTypePsgRougeClick(TObject *Sender);
    
    
    void ButtonAideClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:
    int Numero;
    feu_pieton *f;
    voie *v;
public:
    int x, y;
    int NbFeux, // Nbre de feux liés
        *NumFeux; // Liste des feux liés
    direction Direction;
    TfrmDefFeuxPietons(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    bool RafraichitInfos(); // retourne faux quand il n'a pas pu ajouté le feu
    void RafraichitFeuxPietons();
    void AnalyseEtatFeux(); // v5.2.1
    bool AppliqueChoix();
    void RafraichitInfosFeux();
};
//---------------------------------------------------------------------------
extern TfrmDefFeuxPietons *frmDefFeuxPietons;
//---------------------------------------------------------------------------
#endif
