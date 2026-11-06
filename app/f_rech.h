//---------------------------------------------------------------------------
#ifndef f_rechH
#define f_rechH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h"      // portage
// #include "CSPIN.h" // portage
// #include <Classes.hpp> // portage
// #include <ComCtrls.hpp> // portage
// #include <Controls.hpp> // portage
// #include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TfrmRecherche : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TRadioButton *RadioButtonCoordonnees{};
    TCSpinEdit *CSpinEditX{};
    TCSpinEdit *CSpinEditY{};
    TRadioButton *RadioButtonParking{};
    TLabel *Label1{};
    TCSpinEdit *CSpinEditParking{};
    TRadioButton *RadioButtonFeu{};
    TCSpinEdit *CSpinEditFeu{};
    TButton *ButtonAnnuler{};
    TRadioButton *RadioButtonVehicule{};
    TCSpinEdit *CSpinEditVehicule{};
    TCSpinEdit *CSpinEditPieton{};
    TRadioButton *RadioButtonPieton{};
    TStatusBar *StatusBar{};
    TButton *ButtonAide{};
    TRadioButton *RadioButtonArretBus{};
    TCSpinEdit *CSpinEditArretBus{};
    TCSpinEdit *CSpinEditBus{};
    TRadioButton *RadioButtonBus{};
    TRadioButton *RadioButtonArretTram{};
    TCSpinEdit *CSpinEditArretTram{};
    TRadioButton *RadioButtonTram{};
    TCSpinEdit *CSpinEditTram{};
    TRadioButton *RadioButtonTaxi{};
    TCSpinEdit *CSpinEditTaxi{};
    TRadioButton *RadioButtonPlaceTaxi{};
    TCSpinEdit *CSpinEditPlaceTaxi{};
    TRadioButton *RadioButtonFileTaxi{};
    TCSpinEdit *CSpinEditFileTaxi{};
    TRadioButton *RadioButtonPlacePark{};
    TCSpinEdit *CSpinEditPlacePark{};
    TRadioButton *RadioButtonFilePark{};
    TCSpinEdit *CSpinEditFilePark{};
    TRadioButton *RadioButtonVehlib{};
    TCSpinEdit *CSpinEditVehlib{};
    TRadioButton *RadioButtonPlaceVehlib{};
    TCSpinEdit *CSpinEditPlaceVehlib{};
    void RadioButtonClick(TObject *Sender);
    void FormShow(TObject *Sender) const;
    void CSpinEditXChange(TObject *Sender) const;
    void CSpinEditYChange(TObject *Sender) const;
    void CSpinEditParkingChange(TObject *Sender) const;
    void CSpinEditFeuChange(TObject *Sender) const;

    void CSpinEditVehiculeChange(TObject *Sender) const;
    void ButtonAnnulerClick(TObject *Sender);
    static void FormHide(TObject *Sender);
    void CSpinEditPietonChange(TObject *Sender) const;
    void ButtonAideClick(TObject *Sender);
    void CSpinEditArretBusChange(TObject *Sender) const;
    void CSpinEditBusChange(TObject *Sender) const;
    void CSpinEditArretTramChange(TObject *Sender) const;
    void CSpinEditTramChange(TObject *Sender) const;
    void CSpinEditTaxiChange(TObject *Sender) const;
    void CSpinEditPlaceTaxiChange(TObject *Sender) const;
    void CSpinEditFileTaxiChange(TObject *Sender) const;
    void CSpinEditPlaceParkChange(TObject *Sender) const;
    void CSpinEditFileParkChange(TObject *Sender) const;
    void CSpinEditVehlibChange(TObject *Sender) const;
    void CSpinEditPlaceVehlibChange(TObject *Sender) const;
    void FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);

private: // Déclarations de l'utilisateur
    void RafraichitAffichage();

public: // Déclarations de l'utilisateur
    explicit TfrmRecherche(TComponent *Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmRecherche *frmRecherche;
//---------------------------------------------------------------------------
#endif
