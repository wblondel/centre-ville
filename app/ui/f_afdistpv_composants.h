//---------------------------------------------------------------------------
// f_afdistpv_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_afdistpv.dfm : ne pas modifier
//
// COMPOSANTS_frmAffDistPlacesVehlib(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmAffDistPlacesVehlib(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_afdistpv_composantsH
#define f_afdistpv_composantsH

#define COMPOSANTS_frmAffDistPlacesVehlib(X) \
  X(TStringGrid, StringGridDistances) \

#define EVENEMENTS_frmAffDistPlacesVehlib(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \

#endif
