//---------------------------------------------------------------------------
// f_stats_composants.h - source du portage depuis la 6.0.21 ; généré jusqu'à la 6.0.20 par
// tools/dfm2ui.py depuis src/f_stats.dfm (tag v6.0.20)
//
// COMPOSANTS_frmStatistiques(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmStatistiques(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_stats_composantsH
#define f_stats_composantsH

#define COMPOSANTS_frmStatistiques(X) \
  X(TPaintBox, PaintBoxStats) \

#define EVENEMENTS_frmStatistiques(X) \
  X(this, OnCreate, FormCreate) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(PaintBoxStats, OnPaint, PaintBoxStatsPaint) \

#endif
