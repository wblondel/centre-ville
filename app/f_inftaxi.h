//---------------------------------------------------------------------------
#ifndef f_inftaxiH
#define f_inftaxiH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "simul.h"
//---------------------------------------------------------------------------
class TfrmInfosTaxi : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TButton *ButtonOk;
    TLabel *Label0;
    TLabel *LabelCoordonnees;
    TRadioButton *RadioButtonAucunTaxi;
    TGroupBox *GroupBoxTaxi;
    TRadioButton *RadioButtonTaxiNumero;
    TLabel *Label4;
    TLabel *LabelTeteFileOrigine;
    TLabel *Label6;
    TLabel *LabelParcours;
    TLabel *Label2;
    TLabel *LabelDirection;
    TLabel *Label9;
    TLabel *LabelVitesse;
    TButton *ButtonAide;
    TLabel *Label3;
    TLabel *LabelNumeroFile;
    TLabel *Label5;
    TLabel *LabelNbPersonnes;
    TLabel *Label8;
    TLabel *LabelParcoursDepuisDepart;
    TLabel *Label7;
    TLabel *LabelToursDepuisDepart;
    TLabel *Label1;
    TLabel *LabelSituation;
    TLabel *Label10;
    TLabel *LabelToursDepuisDepot;
    TLabel *Label11;
    TLabel *LabelToursDepuisRetour;
    TLabel *Label12;
    TLabel *LabelDistanceTeteFileDepart;
    void FormShow(TObject *Sender);
    void FormHide(TObject *Sender);
    void ButtonOkClick(TObject *Sender);
    void ButtonAideClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
    voie *v;
    taxi *Taxi;
    int x, y;
    void RendreVisibleInfos(bool Visible);
public:		// Déclarations de l'utilisateur
    TfrmInfosTaxi(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void RafraichitInfos();
};
//---------------------------------------------------------------------------
extern TfrmInfosTaxi *frmInfosTaxi;
//---------------------------------------------------------------------------
#endif
  