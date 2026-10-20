//---------------------------------------------------------------------------
// f_infptn_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_infptn.dfm : ne pas modifier
//
// COMPOSANTS_frmInfosPieton(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmInfosPieton(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_infptn_composantsH
#define f_infptn_composantsH

#define COMPOSANTS_frmInfosPieton(X) \
  X(TLabel, Label0) \
  X(TLabel, LabelCoordonnees) \
  X(TButton, ButtonOk) \
  X(TRadioButton, RadioButtonAucunPieton) \
  X(TGroupBox, GroupBoxPieton) \
  X(TLabel, Label1) \
  X(TLabel, Label3) \
  X(TLabel, Label4) \
  X(TLabel, LabelSituation) \
  X(TLabel, LabelParcours) \
  X(TLabel, LabelDirection) \
  X(TLabel, Label2) \
  X(TLabel, LabelPosition) \
  X(TLabel, Label5) \
  X(TLabel, LabelBusTramTaxiVehlib) \
  X(TLabel, Label6) \
  X(TLabel, LabelArretBusTramFileTaxiVehlib) \
  X(TGroupBox, GroupBoxEpidemie) \
  X(TLabel, Label7) \
  X(TLabel, LabelInfecte) \
  X(TLabel, Label8) \
  X(TLabel, LabelRangs) \
  X(TLabel, Label9) \
  X(TLabel, LabelTours) \
  X(TLabel, Label10) \
  X(TLabel, LabelInfections) \
  X(TLabel, Label11) \
  X(TLabel, LabelPeriodeInfectiosite) \
  X(TLabel, Label13) \
  X(TLabel, LabelChargeVirale) \
  X(TLabel, Label12) \
  X(TLabel, LabelEtatFinal) \
  X(TCheckBox, CheckBoxEpidemie) \
  X(TRadioButton, RadioButtonPietonNumero) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmInfosPieton(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
