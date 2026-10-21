//---------------------------------------------------------------------------
// f_infvehlib_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_infvehlib.dfm : ne pas modifier
//
// COMPOSANTS_frmInfosVehlib(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmInfosVehlib(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_infvehlib_composantsH
#define f_infvehlib_composantsH

#define COMPOSANTS_frmInfosVehlib(X) \
  X(TLabel, Label0) \
  X(TLabel, LabelCoordonnees) \
  X(TButton, ButtonOk) \
  X(TRadioButton, RadioButtonAucunVehlib) \
  X(TGroupBox, GroupBoxVehlib) \
  X(TLabel, Label2) \
  X(TLabel, Label6) \
  X(TLabel, LabelParcours) \
  X(TLabel, LabelDirection) \
  X(TLabel, Label10) \
  X(TLabel, LabelVitesse) \
  X(TLabel, Label3) \
  X(TLabel, LabelNumeroPlaceDepart) \
  X(TLabel, Label8) \
  X(TLabel, LabelParcoursDepuisDepart) \
  X(TLabel, Label7) \
  X(TLabel, LabelToursDepuisDepart) \
  X(TLabel, Label1) \
  X(TLabel, LabelSituation) \
  X(TLabel, Label9) \
  X(TLabel, LabelDistancePlaceArrivee) \
  X(TLabel, Label4) \
  X(TLabel, LabelNumeroPlaceArrivee) \
  X(TLabel, Label5) \
  X(TLabel, LabelNbPersonnes) \
  X(TRadioButton, RadioButtonVehlibNumero) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmInfosVehlib(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
