//---------------------------------------------------------------------------
#ifndef f_infbusH
#define f_infbusH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "simul.h"
//---------------------------------------------------------------------------
class TfrmInfosBus : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TButton *ButtonOk;
    TLabel *Label0;
    TLabel *LabelCoordonnees;
    TRadioButton *RadioButtonAucunBus;
    TGroupBox *GroupBoxBus;
    TRadioButton *RadioButtonBusNumero;
    TLabel *Label3;
    TLabel *Label5;
    TLabel *LabelDernierArret;
    TLabel *LabelTourSortie;
    TLabel *Label7;
    TLabel *LabelParcours;
    TLabel *Label1;
    TLabel *LabelDirection;
    TLabel *Label10;
    TLabel *LabelParcoursEffectue;
    TLabel *Label11;
    TLabel *LabelDetour;
    TLabel *Label12;
    TLabel *Label6;
    TLabel *LabelTours;
    TLabel *Label13;
    TLabel *LabelAttente;
    TLabel *LabelVitesse;
    TButton *ButtonAide;
    TLabel *Label2;
    TLabel *LabelNumeroLigne;
    TLabel *Label8;
    TLabel *LabelDistance;
    TLabel *LabelDistanceMin;
    TLabel *Label4;
    TLabel *LabelProchainArret;
    TLabel *Label9;
    TLabel *Label14;
    TLabel *LabelNbPersonnes;
    TLabel *Label17;
    TLabel *Label18;
    TLabel *Label15;
    TLabel *LabelParcoursEntreArrets;
    TLabel *Label16;
    TLabel *LabelToursEntreArrets;
    void FormShow(TObject *Sender);
    void FormHide(TObject *Sender);
    void ButtonOkClick(TObject *Sender);
    void ButtonAideClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
    voie *v;
    bus *Bus;
    int x, y;
    void RendreVisibleInfos(bool Visible);
public:		// Déclarations de l'utilisateur
    TfrmInfosBus(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void RafraichitInfos();
};
//---------------------------------------------------------------------------
extern TfrmInfosBus *frmInfosBus;
//---------------------------------------------------------------------------
#endif
  