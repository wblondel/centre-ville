//---------------------------------------------------------------------------
// f_epidemie_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_epidemie.dfm : ne pas modifier
//
// COMPOSANTS_frmEpidemie(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmEpidemie(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_epidemie_composantsH
#define f_epidemie_composantsH

#define COMPOSANTS_frmEpidemie(X) \
  X(TLabel, Label7) \
  X(TLabel, Label8) \
  X(TGroupBox, GroupBox) \
  X(TLabel, Label1) \
  X(TLabel, Label2) \
  X(TLabel, Label3) \
  X(TLabel, Label4) \
  X(TLabel, Label5) \
  X(TLabel, Label6) \
  X(TCSpinEdit, CSpinEditInfectiosite) \
  X(TCSpinEdit, CSpinEditPatientZero) \
  X(TCSpinEdit, CSpinEditChargeViraleFatale) \
  X(TCheckBox, CheckBoxReinfections) \
  X(TButton, ButtonOk) \
  X(TButton, ButtonAnnuler) \
  X(TButton, ButtonAide) \
  X(TCheckBox, CheckBoxActiver) \

#define EVENEMENTS_frmEpidemie(X) \
  X(this, OnShow, FormShow) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
