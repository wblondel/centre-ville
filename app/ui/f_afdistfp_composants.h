//---------------------------------------------------------------------------
// f_afdistfp_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_afdistfp.dfm : ne pas modifier
//
// COMPOSANTS_frmAffDistFilesPark(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmAffDistFilesPark(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_afdistfp_composantsH
#define f_afdistfp_composantsH

#define COMPOSANTS_frmAffDistFilesPark(X) \
  X(TStringGrid, StringGridDistances) \
  X(TPanel, Panel) \
  X(TCheckBox, CheckBoxCouloirsBus) \

#define EVENEMENTS_frmAffDistFilesPark(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(CheckBoxCouloirsBus, OnClick, CheckBoxCouloirsBusClick) \

#endif
