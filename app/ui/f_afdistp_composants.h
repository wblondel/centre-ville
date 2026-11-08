//---------------------------------------------------------------------------
// f_afdistp_composants.h - source du portage depuis la 6.0.21 ; généré jusqu'à la 6.0.20 par
// tools/dfm2ui.py depuis src/f_afdistp.dfm (tag v6.0.20)
//
// COMPOSANTS_frmAffDistParkings(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmAffDistParkings(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_afdistp_composantsH
#define f_afdistp_composantsH

#define COMPOSANTS_frmAffDistParkings(X) \
  X(TStringGrid, StringGridDistances) \
  X(TPanel, Panel) \
  X(TCheckBox, CheckBoxCouloirsBus) \

#define EVENEMENTS_frmAffDistParkings(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(CheckBoxCouloirsBus, OnClick, CheckBoxCouloirsBusClick) \

#endif
