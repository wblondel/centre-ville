//---------------------------------------------------------------------------
#ifndef f_rechH
#define f_rechH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include "CSPIN.h" // portage
//#include <Classes.hpp> // portage
//#include <ComCtrls.hpp> // portage
//#include <Controls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "base.h"
//---------------------------------------------------------------------------
class TfrmRecherche : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
  TRadioButton *RadioButtonCoordonnees;
  TCSpinEdit *CSpinEditX;
  TCSpinEdit *CSpinEditY;
  TRadioButton *RadioButtonParking;
  TLabel *Label1;
  TCSpinEdit *CSpinEditParking;
  TRadioButton *RadioButtonFeu;
  TCSpinEdit *CSpinEditFeu;
  TButton *ButtonAnnuler;
  TRadioButton *RadioButtonVehicule;
  TCSpinEdit *CSpinEditVehicule;
  TCSpinEdit *CSpinEditPieton;
  TRadioButton *RadioButtonPieton;
  TStatusBar *StatusBar;
  TButton *ButtonAide;
  TRadioButton *RadioButtonArretBus;
  TCSpinEdit *CSpinEditArretBus;
  TCSpinEdit *CSpinEditBus;
  TRadioButton *RadioButtonBus;
  TRadioButton *RadioButtonArretTram;
  TCSpinEdit *CSpinEditArretTram;
  TRadioButton *RadioButtonTram;
  TCSpinEdit *CSpinEditTram;
  TRadioButton *RadioButtonTaxi;
  TCSpinEdit *CSpinEditTaxi;
  TRadioButton *RadioButtonPlaceTaxi;
  TCSpinEdit *CSpinEditPlaceTaxi;
  TRadioButton *RadioButtonFileTaxi;
  TCSpinEdit *CSpinEditFileTaxi;
  TRadioButton *RadioButtonPlacePark;
  TCSpinEdit *CSpinEditPlacePark;
  TRadioButton *RadioButtonFilePark;
  TCSpinEdit *CSpinEditFilePark;
  TRadioButton *RadioButtonVehlib;
  TCSpinEdit *CSpinEditVehlib;
  TRadioButton *RadioButtonPlaceVehlib;
  TCSpinEdit *CSpinEditPlaceVehlib;
  void RadioButtonClick(TObject *Sender);
  void FormShow(TObject *Sender);
  void CSpinEditXChange(TObject *Sender);
  void CSpinEditYChange(TObject *Sender);
  void CSpinEditParkingChange(TObject *Sender);
  void CSpinEditFeuChange(TObject *Sender);

  void CSpinEditVehiculeChange(TObject *Sender);
  void ButtonAnnulerClick(TObject *Sender);
  void FormHide(TObject *Sender);
  void CSpinEditPietonChange(TObject *Sender);
  void ButtonAideClick(TObject *Sender);
  void CSpinEditArretBusChange(TObject *Sender);
  void CSpinEditBusChange(TObject *Sender);
  void CSpinEditArretTramChange(TObject *Sender);
  void CSpinEditTramChange(TObject *Sender);
  void CSpinEditTaxiChange(TObject *Sender);
  void CSpinEditPlaceTaxiChange(TObject *Sender);
  void CSpinEditFileTaxiChange(TObject *Sender);
  void CSpinEditPlaceParkChange(TObject *Sender);
  void CSpinEditFileParkChange(TObject *Sender);
  void CSpinEditVehlibChange(TObject *Sender);
  void CSpinEditPlaceVehlibChange(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
    int xPrc, yPrc;
    void RafraichitAffichage();
public:		// Déclarations de l'utilisateur
    TfrmRecherche(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmRecherche *frmRecherche;
//---------------------------------------------------------------------------
#endif
