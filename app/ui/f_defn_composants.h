//---------------------------------------------------------------------------
// f_defn_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_defn.dfm : ne pas modifier
//
// COMPOSANTS_frmDefNature(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmDefNature(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_defn_composantsH
#define f_defn_composantsH

#define COMPOSANTS_frmDefNature(X) \
  X(TLabel, LabelRq) \
  X(TRadioGroup, RadioGroupNature) \
  X(TCheckBox, CheckBoxEcraser) \
  X(TButton, ButtonAppliquer) \
  X(TButton, ButtonFermer) \
  X(TButton, ButtonAide) \
  X(TCheckBox, CheckBoxPelouse) \

#define EVENEMENTS_frmDefNature(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(RadioGroupNature, OnClick, RadioGroupNatureClick) \
  X(ButtonAppliquer, OnClick, ButtonAppliquerClick) \
  X(ButtonFermer, OnClick, ButtonFermerClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
