//---------------------------------------------------------------------------
// f_genres_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_genres.dfm : ne pas modifier
//
// COMPOSANTS_frmGeneration(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmGeneration(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_genres_composantsH
#define f_genres_composantsH

#define COMPOSANTS_frmGeneration(X) \
  X(TLabel, LabelRq) \
  X(TGroupBox, GroupBox) \
  X(TLabel, Label1) \
  X(TLabel, Label2) \
  X(TLabel, Label3) \
  X(TLabel, Label4) \
  X(TLabel, Label5) \
  X(TLabel, Label6) \
  X(TTrackBar, TrackBarTailleMinSegments) \
  X(TTrackBar, TrackBarProbaSortieIntersection) \
  X(TTrackBar, TrackBarTailleMaxSegments) \
  X(TTrackBar, TrackBarProbaEntreeIntersection) \
  X(TTrackBar, TrackBarProbaTraverseeRoute) \
  X(TButton, ButtonAnnuler) \
  X(TButton, ButtonOk) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmGeneration(X) \
  X(this, OnKeyDown, FormKeyDown) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
