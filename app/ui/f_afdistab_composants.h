//---------------------------------------------------------------------------
// f_afdistab_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_afdistab.dfm : ne pas modifier
//
// COMPOSANTS_frmAffDistArretsBus(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmAffDistArretsBus(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_afdistab_composantsH
#define f_afdistab_composantsH

#define COMPOSANTS_frmAffDistArretsBus(X) \
  X(TStringGrid, StringGridDistances) \

#define EVENEMENTS_frmAffDistArretsBus(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \

#endif
