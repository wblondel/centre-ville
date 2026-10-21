//---------------------------------------------------------------------------
#ifndef F_deffH
#define F_deffH
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
class TfrmDefFeux : public TForm
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
    TLabel *Label6;
    TLabel *LabelNbFeuxDmd;
    TSpeedButton *SpeedButtonSelFeuxDmd;
    TLabel *Label7;
    TLabel *LabelNbFeuxAtt;
    TSpeedButton *SpeedButtonSelFeuxAtt;
    TButton *ButtonAide;
    TLabel *Label8;
    TLabel *LabelFeuxPietons;
    void FormShow(TObject *Sender);
    void ButtonOkClick(TObject *Sender);
    
    void RadioGroupEtatClick(TObject *Sender);
    void RadioGroupTypeDmdVertClick(TObject *Sender);
    
    void CSpinEditDureeVertChange(TObject *Sender);
    void ButtonAppliquerClick(TObject *Sender);
    void ButtonAnnulerClick(TObject *Sender);
    
    void SpeedButtonSelFeuxDmdClick(TObject *Sender);
    void FormHide(TObject *Sender);
    void SpeedButtonSelFeuxAttClick(TObject *Sender);
    void RadioGroupTypePsgRougeClick(TObject *Sender);
    
    
    void ButtonAideClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:
    int Numero;
    feu *f;
    voie *v;
public:
    int x, y;
    int NbFeuxDmd, // Nbre de feux demandant le rouge
        NbFeuxAtt,// Nbre de feux devant être rouge pour que Numero passe au vert
        *NumFeuxDmd, // Liste des feux demandant le rouge
        *NumFeuxAtt; // Liste des feux devant être rouge pour que Numero passe au vert
    direction Direction;
    TfrmDefFeux(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    bool RafraichitInfos(); // retourne faux quand il n'a pas pu ajouter le feu
    bool AppliqueChoix();
    void RafraichitInfosFeuxDmd();
    void RafraichitInfosFeuxAtt();
};
//---------------------------------------------------------------------------
extern TfrmDefFeux *frmDefFeux;
//---------------------------------------------------------------------------
#endif
 