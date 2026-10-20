//---------------------------------------------------------------------------
// f_prmsim_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_prmsim.dfm : ne pas modifier
//
// COMPOSANTS_frmParamSimul(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmParamSimul(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_prmsim_composantsH
#define f_prmsim_composantsH

#define COMPOSANTS_frmParamSimul(X) \
  X(TGroupBox, GroupBoxNbElements) \
  X(TLabel, LabelNbVehicules) \
  X(TLabel, LabelNbPietons) \
  X(TLabel, Label1) \
  X(TLabel, LabelNbArretsBus) \
  X(TLabel, LabelNbArretsTram) \
  X(TLabel, LabelNbPlacesTaxi) \
  X(TLabel, Label2) \
  X(TCSpinEdit, CSpinEditNbVehicules) \
  X(TCSpinEdit, CSpinEditNbPietons) \
  X(TBitBtn, BitBtnDefLignesBus) \
  X(TBitBtn, BitBtnDefLignesTram) \
  X(TBitBtn, BitBtnDefTaxis) \
  X(TBitBtn, BitBtnDefVehlibs) \
  X(TGroupBox, GroupBoxFonctionnement) \
  X(TLabel, LabelTours) \
  X(TLabel, LabelAttenteAvChx) \
  X(TLabel, LabelNbToursParSeconde) \
  X(TLabel, LabelDureeOrange) \
  X(TLabel, LabelDureeRouge) \
  X(TLabel, LabelDureeVert) \
  X(TLabel, Label7) \
  X(TCSpinEdit, CSpinEditAttenteAvChx) \
  X(TCSpinEdit, CSpinEditNbToursParSeconde) \
  X(TCSpinEdit, CSpinEditDureeOrange) \
  X(TCSpinEdit, CSpinEditDureeRouge) \
  X(TCSpinEdit, CSpinEditDureeVert) \
  X(TBitBtn, BitBtnAppliqueDureeFeuVert) \
  X(TBitBtn, BitBtnAppliqueDureeFeuRouge) \
  X(TRadioGroup, RadioGroupVitesses) \
  X(TGroupBox, GroupBoxPietons) \
  X(TLabel, LabelNbCasesMaxPsgPtn) \
  X(TLabel, Label4) \
  X(TLabel, LabelProbaPsgPt) \
  X(TLabel, Label6) \
  X(TCSpinEdit, CSpinEditNbCasesMaxPsgPtn) \
  X(TCSpinEdit, CSpinEditProbaPsgPt) \
  X(TButton, ButtonOk) \
  X(TButton, ButtonAnnuler) \
  X(TButton, ButtonAppliquer) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmParamSimul(X) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(CSpinEditNbVehicules, OnChange, CSpinEditNbVehiculesChange) \
  X(CSpinEditNbPietons, OnChange, CSpinEditNbPietonsChange) \
  X(BitBtnDefLignesBus, OnClick, BitBtnDefLignesBusClick) \
  X(BitBtnDefLignesTram, OnClick, BitBtnDefLignesTramClick) \
  X(BitBtnDefTaxis, OnClick, BitBtnDefTaxisClick) \
  X(BitBtnDefVehlibs, OnClick, BitBtnDefVehlibsClick) \
  X(CSpinEditAttenteAvChx, OnChange, Change) \
  X(CSpinEditNbToursParSeconde, OnChange, Change) \
  X(CSpinEditDureeOrange, OnChange, Change) \
  X(CSpinEditDureeRouge, OnChange, CSpinEditDureeRougeChange) \
  X(CSpinEditDureeVert, OnChange, CSpinEditDureeVertChange) \
  X(BitBtnAppliqueDureeFeuVert, OnClick, BitBtnAppliqueDureeFeuVertClick) \
  X(BitBtnAppliqueDureeFeuRouge, OnClick, BitBtnAppliqueDureeFeuRougeClick) \
  X(RadioGroupVitesses, OnClick, Change) \
  X(CSpinEditNbCasesMaxPsgPtn, OnChange, Change) \
  X(CSpinEditProbaPsgPt, OnChange, Change) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAppliquer, OnClick, ButtonAppliquerClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
