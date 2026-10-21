//---------------------------------------------------------------------------
#ifndef f_infvehlibH
#define f_infvehlibH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "simul.h"
//---------------------------------------------------------------------------
class TfrmInfosVehlib : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TButton *ButtonOk;
    TLabel *Label0;
    TLabel *LabelCoordonnees;
    TRadioButton *RadioButtonAucunVehlib;
    TGroupBox *GroupBoxVehlib;
    TRadioButton *RadioButtonVehlibNumero;
    TLabel *Label6;
    TLabel *LabelParcours;
    TLabel *Label2;
    TLabel *LabelDirection;
        TLabel *Label10;
    TLabel *LabelVitesse;
    TButton *ButtonAide;
    TLabel *Label3;
        TLabel *LabelNumeroPlaceDepart;
    TLabel *Label8;
    TLabel *LabelParcoursDepuisDepart;
    TLabel *Label7;
    TLabel *LabelToursDepuisDepart;
    TLabel *Label1;
    TLabel *LabelSituation;
        TLabel *Label9;
        TLabel *LabelDistancePlaceArrivee;
        TLabel *Label4;
        TLabel *LabelNumeroPlaceArrivee;
        TLabel *Label5;
    TLabel *LabelNbPersonnes;
    void FormShow(TObject *Sender);
    void FormHide(TObject *Sender);
    void ButtonOkClick(TObject *Sender);
    void ButtonAideClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
    voie *v;
    vehlib *Vehlib;
    int x, y;
    void RendreVisibleInfos(bool Visible);
public:		// Déclarations de l'utilisateur
    TfrmInfosVehlib(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void RafraichitInfos();
};
//---------------------------------------------------------------------------
extern TfrmInfosVehlib *frmInfosVehlib;
//---------------------------------------------------------------------------
#endif

