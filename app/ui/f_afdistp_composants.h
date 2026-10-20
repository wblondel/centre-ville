//---------------------------------------------------------------------------
// f_afdistp_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_afdistp.dfm : ne pas modifier
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
