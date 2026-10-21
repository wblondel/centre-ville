//---------------------------------------------------------------------------
#ifndef f_inftramH
#define f_inftramH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "simul.h"
//---------------------------------------------------------------------------
class TfrmInfosTram : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TButton *ButtonOk;
    TLabel *Label0;
    TLabel *LabelCoordonnees;
    TRadioButton *RadioButtonAucunTram;
    TGroupBox *GroupBoxTram;
    TRadioButton *RadioButtonTramNumero;
    TLabel *Label3;
    TLabel *LabelDernierArret;
  TLabel *Label6;
    TLabel *LabelParcours;
    TLabel *Label1;
    TLabel *LabelDirection;
  TLabel *Label9;
    TLabel *LabelParcoursEffectue;
  TLabel *Label10;
    TLabel *LabelDetour;
  TLabel *Label11;
  TLabel *Label5;
    TLabel *LabelTours;
  TLabel *Label12;
    TLabel *LabelAttente;
    TLabel *LabelVitesse;
    TButton *ButtonAide;
    TLabel *Label2;
    TLabel *LabelNumeroLigne;
  TLabel *Label7;
    TLabel *LabelDistance;
    TLabel *LabelDistanceMin;
    TLabel *Label4;
    TLabel *LabelProchainArret;
  TLabel *Label8;
  TLabel *Label13;
    TLabel *LabelNbPersonnes;
    TLabel *Label17;
    TLabel *Label18;
  TLabel *Label14;
    TLabel *LabelParcoursEntreArrets;
  TLabel *Label15;
    TLabel *LabelToursEntreArrets;
    void FormShow(TObject *Sender);
    void FormHide(TObject *Sender);
    void ButtonOkClick(TObject *Sender);
    void ButtonAideClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
    voie *v;
    tram *Tram;
    int x, y;
    void RendreVisibleInfos(bool Visible);
public:		// Déclarations de l'utilisateur
    TfrmInfosTram(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void RafraichitInfos();
};
//---------------------------------------------------------------------------
extern TfrmInfosTram *frmInfosTram;
//---------------------------------------------------------------------------
#endif
