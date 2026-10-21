//---------------------------------------------------------------------------
// f_afdistft_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_afdistft.dfm : ne pas modifier
//
// COMPOSANTS_frmAffDistFilesTaxi(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmAffDistFilesTaxi(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_afdistft_composantsH
#define f_afdistft_composantsH

#define COMPOSANTS_frmAffDistFilesTaxi(X) \
  X(TStringGrid, StringGridDistances) \

#define EVENEMENTS_frmAffDistFilesTaxi(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \

#endif
