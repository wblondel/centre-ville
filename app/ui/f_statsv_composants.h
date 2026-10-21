//---------------------------------------------------------------------------
// f_statsv_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_statsv.dfm : ne pas modifier
//
// COMPOSANTS_frmStatsVehicules(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmStatsVehicules(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_statsv_composantsH
#define f_statsv_composantsH

#define COMPOSANTS_frmStatsVehicules(X) \
  X(TStringGrid, StringGridCoordParkings) \
  X(TButton, ButtonFermer) \
  X(TTabControl, TabControl) \
  X(TStringGrid, StringGridStatsVehicules) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmStatsVehicules(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(ButtonFermer, OnClick, ButtonFermerClick) \
  X(TabControl, OnChange, TabControlChange) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
