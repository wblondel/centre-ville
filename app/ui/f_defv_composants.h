//---------------------------------------------------------------------------
// f_defv_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_defv.dfm : ne pas modifier
//
// COMPOSANTS_frmDefVoie(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmDefVoie(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_defv_composantsH
#define f_defv_composantsH

#define COMPOSANTS_frmDefVoie(X) \
  X(TLabel, Label1) \
  X(TLabel, LabelCoordonnees) \
  X(TRadioGroup, RadioGroupPrioriteNord) \
  X(TRadioGroup, RadioGroupPrioriteOuest) \
  X(TRadioGroup, RadioGroupPrioriteEst) \
  X(TRadioGroup, RadioGroupPrioriteSud) \
  X(TButton, ButtonOk) \
  X(TButton, ButtonAnnuler) \
  X(TBitBtn, BitBtnDefFeuN) \
  X(TBitBtn, BitBtnDefFeuO) \
  X(TBitBtn, BitBtnDefFeuE) \
  X(TBitBtn, BitBtnDefFeuS) \
  X(TButton, ButtonAppliquer) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmDefVoie(X) \
  X(this, OnCreate, FormCreate) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(RadioGroupPrioriteNord, OnClick, RadioGroupPrioriteClick) \
  X(RadioGroupPrioriteOuest, OnClick, RadioGroupPrioriteClick) \
  X(RadioGroupPrioriteEst, OnClick, RadioGroupPrioriteClick) \
  X(RadioGroupPrioriteSud, OnClick, RadioGroupPrioriteClick) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAnnuler, OnClick, ButtonAnnulerClick) \
  X(BitBtnDefFeuN, OnClick, BitBtnDefFeuClick) \
  X(BitBtnDefFeuO, OnClick, BitBtnDefFeuClick) \
  X(BitBtnDefFeuE, OnClick, BitBtnDefFeuClick) \
  X(BitBtnDefFeuS, OnClick, BitBtnDefFeuClick) \
  X(ButtonAppliquer, OnClick, ButtonAppliquerClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
