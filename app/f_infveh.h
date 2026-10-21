//---------------------------------------------------------------------------
#ifndef f_infvehH
#define f_infvehH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "simul.h"
//v5.0 ----------------------------------------------------------------------
#define DEPART   0
#define ARRIVEE  1
#define PARKING  0
#define FILEPARK 1
//---------------------------------------------------------------------------
class TfrmInfosVehicule : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TButton *ButtonOk;
    TLabel *Label0;
    TLabel *LabelCoordonnees;
    TRadioButton *RadioButtonAucunVehicule;
    TGroupBox *GroupBoxVehicule;
    TRadioButton *RadioButtonVehiculeNumero;
  TLabel *LabelDepart;
  TLabel *LabelArrivee;
    TLabel *Label4;
    TLabel *Label7;
  TLabel *LabelParkingDepart;
  TLabel *LabelParkingArrivee;
    TLabel *LabelTourSortie;
    TLabel *LabelDistance;
    TLabel *Label6;
    TLabel *LabelParcours;
  TLabel *Label3;
    TLabel *LabelDirection;
    TLabel *Label8;
    TLabel *LabelDistanceMin;
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
    TLabel *Label13;
    TLabel *LabelNivPriorite;
    TButton *ButtonAide;
    void FormShow(TObject *Sender);
    void FormHide(TObject *Sender);
    void ButtonOkClick(TObject *Sender);
    void ButtonAideClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
    voie *v;
    vehicule *Veh;
    int x, y;
    void RendreVisibleInfos(bool Visible);
public:		// Déclarations de l'utilisateur
    TfrmInfosVehicule(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void RafraichitInfos();
};
//---------------------------------------------------------------------------
extern TfrmInfosVehicule *frmInfosVehicule;
//---------------------------------------------------------------------------
#endif
  