//---------------------------------------------------------------------------
// f_dimzone_composants.h - source du portage depuis la 6.0.21 ; généré jusqu'à la 6.0.20 par
// tools/dfm2ui.py depuis src/f_dimzone.dfm (tag v6.0.20)
//
// COMPOSANTS_frmDimensionsZone(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmDimensionsZone(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_dimzone_composantsH
#define f_dimzone_composantsH

#define COMPOSANTS_frmDimensionsZone(X) \
  X(TLabel, Label1) \
  X(TLabel, Label2) \
  X(TCSpinEdit, CSpinEditNbX) \
  X(TCSpinEdit, CSpinEditNbY) \
  X(TButton, ButtonCreer) \
  X(TButton, ButtonAnnuler) \
  X(TButton, ButtonAide) \
  X(TCheckBox, CheckBoxAfficherParametres) \
  X(TCheckBox, CheckBoxGenererReseau) \

#define EVENEMENTS_frmDimensionsZone(X) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnKeyPress, FormKeyPress) \
  X(this, OnShow, FormShow) \
  X(ButtonCreer, OnClick, ButtonCreerClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
