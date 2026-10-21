//---------------------------------------------------------------------------
// f_propos_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_propos.dfm : ne pas modifier
//
// COMPOSANTS_frmAPropos(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmAPropos(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_propos_composantsH
#define f_propos_composantsH

#define COMPOSANTS_frmAPropos(X) \
  X(TLabel, LabelAuteur2) \
  X(TLabel, LabelCV2) \
  X(TLabel, LabelCV1) \
  X(TLabel, LabelAuteur1) \
  X(TLabel, LabelEMail) \
  X(TLabel, LabelURL) \
  X(TLabel, LabelTexte) \
  X(TSpeedButton, SpeedButtonOk) \
  X(TLabel, LabelSousVersion) \
  X(TLabel, LabelGPL1) \
  X(TLabel, LabelGPL2) \
  X(TImage, ImageVersion) \
  X(TLabel, LabelAlignement) \
  X(TImage, ImageEditionSpeciale) \
  X(TLabel, LabelHDPI) \
  X(TTimer, Timer) \
  X(TImageCollection, ImageCollection) \
  X(TVirtualImageList, VirtualImageList) \

#define EVENEMENTS_frmAPropos(X) \
  X(this, OnClick, LabelClick) \
  X(this, OnCreate, FormCreate) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyPress, FormKeyPress) \
  X(this, OnShow, FormShow) \
  X(LabelCV1, OnClick, LabelClick) \
  X(LabelAuteur1, OnClick, LabelClick) \
  X(LabelEMail, OnClick, LabelEMailClick) \
  X(LabelURL, OnClick, LabelURLClick) \
  X(LabelTexte, OnClick, LabelClick) \
  X(SpeedButtonOk, OnClick, LabelClick) \
  X(LabelSousVersion, OnClick, LabelClick) \
  X(LabelGPL2, OnClick, LabelClick) \
  X(ImageVersion, OnClick, LabelClick) \
  X(Timer, OnTimer, TimerTimer) \

#endif
