//---------------------------------------------------------------------------
// f_dimzone_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_dimzone.dfm : ne pas modifier
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
