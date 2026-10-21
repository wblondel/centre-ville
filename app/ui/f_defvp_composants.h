//---------------------------------------------------------------------------
// f_defvp_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_defvp.dfm : ne pas modifier
//
// COMPOSANTS_frmDefVoiePietons(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmDefVoiePietons(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_defvp_composantsH
#define f_defvp_composantsH

#define COMPOSANTS_frmDefVoiePietons(X) \
  X(TLabel, Label1) \
  X(TLabel, LabelCoordonnees) \
  X(TImage, ImageEE) \
  X(TImage, ImageOO) \
  X(TImage, ImageSS) \
  X(TImage, ImageNN) \
  X(TImage, ImageNE) \
  X(TImage, ImageSE) \
  X(TImage, ImageSO) \
  X(TImage, ImageNO) \
  X(TImage, ImageS) \
  X(TImage, ImageO) \
  X(TImage, ImageN) \
  X(TImage, ImageE) \
  X(TImage, Image0) \
  X(TSpeedButton, SpeedButtonN) \
  X(TSpeedButton, SpeedButtonE) \
  X(TSpeedButton, SpeedButtonO) \
  X(TSpeedButton, SpeedButtonS) \
  X(TLabel, Label2) \
  X(TImage, ImageAttention) \
  X(TButton, ButtonOk) \
  X(TButton, ButtonAnnuler) \
  X(TButton, ButtonAppliquer) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmDefVoiePietons(X) \
  X(this, OnCreate, FormCreate) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(SpeedButtonN, OnClick, SpeedButtonClick) \
  X(SpeedButtonE, OnClick, SpeedButtonClick) \
  X(SpeedButtonO, OnClick, SpeedButtonClick) \
  X(SpeedButtonS, OnClick, SpeedButtonClick) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAnnuler, OnClick, ButtonAnnulerClick) \
  X(ButtonAppliquer, OnClick, ButtonAppliquerClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
