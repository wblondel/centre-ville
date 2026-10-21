//---------------------------------------------------------------------------
// f_deff_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_deff.dfm : ne pas modifier
//
// COMPOSANTS_frmDefFeux(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmDefFeux(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_deff_composantsH
#define f_deff_composantsH

#define COMPOSANTS_frmDefFeux(X) \
  X(TLabel, Label1) \
  X(TLabel, LabelCoordonnees) \
  X(TLabel, Label2) \
  X(TLabel, LabelDirection) \
  X(TLabel, Label3) \
  X(TLabel, LabelNumero) \
  X(TLabel, Label6) \
  X(TLabel, LabelNbFeuxDmd) \
  X(TSpeedButton, SpeedButtonSelFeuxDmd) \
  X(TLabel, Label7) \
  X(TLabel, LabelNbFeuxAtt) \
  X(TSpeedButton, SpeedButtonSelFeuxAtt) \
  X(TLabel, Label8) \
  X(TLabel, LabelFeuxPietons) \
  X(TRadioGroup, RadioGroupTypeDmdVert) \
  X(TRadioGroup, RadioGroupEtat) \
  X(TGroupBox, GroupBoxDureeFeu) \
  X(TLabel, Label5) \
  X(TLabel, Label4) \
  X(TCSpinEdit, CSpinEditDureeRouge) \
  X(TCSpinEdit, CSpinEditDureeVert) \
  X(TButton, ButtonOk) \
  X(TButton, ButtonAnnuler) \
  X(TButton, ButtonAppliquer) \
  X(TRadioGroup, RadioGroupTypePsgRouge) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmDefFeux(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(SpeedButtonSelFeuxDmd, OnClick, SpeedButtonSelFeuxDmdClick) \
  X(SpeedButtonSelFeuxAtt, OnClick, SpeedButtonSelFeuxAttClick) \
  X(RadioGroupTypeDmdVert, OnClick, RadioGroupTypeDmdVertClick) \
  X(RadioGroupEtat, OnClick, RadioGroupEtatClick) \
  X(CSpinEditDureeRouge, OnChange, CSpinEditDureeVertChange) \
  X(CSpinEditDureeVert, OnChange, CSpinEditDureeVertChange) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAnnuler, OnClick, ButtonAnnulerClick) \
  X(ButtonAppliquer, OnClick, ButtonAppliquerClick) \
  X(RadioGroupTypePsgRouge, OnClick, RadioGroupTypePsgRougeClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
