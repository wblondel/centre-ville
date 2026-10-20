//---------------------------------------------------------------------------
// f_rech_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_rech.dfm : ne pas modifier
//
// COMPOSANTS_frmRecherche(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmRecherche(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_rech_composantsH
#define f_rech_composantsH

#define COMPOSANTS_frmRecherche(X) \
  X(TLabel, Label1) \
  X(TRadioButton, RadioButtonCoordonnees) \
  X(TCSpinEdit, CSpinEditX) \
  X(TCSpinEdit, CSpinEditY) \
  X(TRadioButton, RadioButtonParking) \
  X(TCSpinEdit, CSpinEditParking) \
  X(TRadioButton, RadioButtonFeu) \
  X(TCSpinEdit, CSpinEditFeu) \
  X(TButton, ButtonAnnuler) \
  X(TRadioButton, RadioButtonVehicule) \
  X(TCSpinEdit, CSpinEditVehicule) \
  X(TCSpinEdit, CSpinEditPieton) \
  X(TRadioButton, RadioButtonPieton) \
  X(TStatusBar, StatusBar) \
  X(TButton, ButtonAide) \
  X(TRadioButton, RadioButtonArretBus) \
  X(TCSpinEdit, CSpinEditArretBus) \
  X(TCSpinEdit, CSpinEditBus) \
  X(TRadioButton, RadioButtonBus) \
  X(TRadioButton, RadioButtonArretTram) \
  X(TCSpinEdit, CSpinEditArretTram) \
  X(TRadioButton, RadioButtonTram) \
  X(TCSpinEdit, CSpinEditTram) \
  X(TRadioButton, RadioButtonTaxi) \
  X(TCSpinEdit, CSpinEditTaxi) \
  X(TRadioButton, RadioButtonPlaceTaxi) \
  X(TCSpinEdit, CSpinEditPlaceTaxi) \
  X(TRadioButton, RadioButtonFileTaxi) \
  X(TCSpinEdit, CSpinEditFileTaxi) \
  X(TRadioButton, RadioButtonPlacePark) \
  X(TCSpinEdit, CSpinEditPlacePark) \
  X(TRadioButton, RadioButtonFilePark) \
  X(TCSpinEdit, CSpinEditFilePark) \
  X(TRadioButton, RadioButtonVehlib) \
  X(TCSpinEdit, CSpinEditVehlib) \
  X(TRadioButton, RadioButtonPlaceVehlib) \
  X(TCSpinEdit, CSpinEditPlaceVehlib) \

#define EVENEMENTS_frmRecherche(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(RadioButtonCoordonnees, OnClick, RadioButtonClick) \
  X(CSpinEditX, OnChange, CSpinEditXChange) \
  X(CSpinEditY, OnChange, CSpinEditYChange) \
  X(RadioButtonParking, OnClick, RadioButtonClick) \
  X(CSpinEditParking, OnChange, CSpinEditParkingChange) \
  X(RadioButtonFeu, OnClick, RadioButtonClick) \
  X(CSpinEditFeu, OnChange, CSpinEditFeuChange) \
  X(ButtonAnnuler, OnClick, ButtonAnnulerClick) \
  X(RadioButtonVehicule, OnClick, RadioButtonClick) \
  X(CSpinEditVehicule, OnChange, CSpinEditVehiculeChange) \
  X(CSpinEditPieton, OnChange, CSpinEditPietonChange) \
  X(RadioButtonPieton, OnClick, RadioButtonClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \
  X(RadioButtonArretBus, OnClick, RadioButtonClick) \
  X(CSpinEditArretBus, OnChange, CSpinEditArretBusChange) \
  X(CSpinEditBus, OnChange, CSpinEditBusChange) \
  X(RadioButtonBus, OnClick, RadioButtonClick) \
  X(RadioButtonArretTram, OnClick, RadioButtonClick) \
  X(CSpinEditArretTram, OnChange, CSpinEditArretTramChange) \
  X(RadioButtonTram, OnClick, RadioButtonClick) \
  X(CSpinEditTram, OnChange, CSpinEditTramChange) \
  X(RadioButtonTaxi, OnClick, RadioButtonClick) \
  X(CSpinEditTaxi, OnChange, CSpinEditTaxiChange) \
  X(RadioButtonPlaceTaxi, OnClick, RadioButtonClick) \
  X(CSpinEditPlaceTaxi, OnChange, CSpinEditPlaceTaxiChange) \
  X(RadioButtonFileTaxi, OnClick, RadioButtonClick) \
  X(CSpinEditFileTaxi, OnChange, CSpinEditFileTaxiChange) \
  X(RadioButtonPlacePark, OnClick, RadioButtonClick) \
  X(CSpinEditPlacePark, OnChange, CSpinEditPlaceParkChange) \
  X(RadioButtonFilePark, OnClick, RadioButtonClick) \
  X(CSpinEditFilePark, OnChange, CSpinEditFileParkChange) \
  X(RadioButtonVehlib, OnClick, RadioButtonClick) \
  X(CSpinEditVehlib, OnChange, CSpinEditVehlibChange) \
  X(RadioButtonPlaceVehlib, OnClick, RadioButtonClick) \
  X(CSpinEditPlaceVehlib, OnChange, CSpinEditPlaceVehlibChange) \

#endif
