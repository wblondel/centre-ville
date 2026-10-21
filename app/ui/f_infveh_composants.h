//---------------------------------------------------------------------------
// f_infveh_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_infveh.dfm : ne pas modifier
//
// COMPOSANTS_frmInfosVehicule(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmInfosVehicule(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_infveh_composantsH
#define f_infveh_composantsH

#define COMPOSANTS_frmInfosVehicule(X) \
  X(TLabel, Label0) \
  X(TLabel, LabelCoordonnees) \
  X(TButton, ButtonOk) \
  X(TRadioButton, RadioButtonAucunVehicule) \
  X(TGroupBox, GroupBoxVehicule) \
  X(TLabel, Label3) \
  X(TLabel, LabelDepart) \
  X(TLabel, LabelArrivee) \
  X(TLabel, Label4) \
  X(TLabel, Label5) \
  X(TLabel, Label6) \
  X(TLabel, Label7) \
  X(TLabel, Label8) \
  X(TLabel, Label9) \
  X(TLabel, Label10) \
  X(TLabel, Label11) \
  X(TLabel, LabelParkingDepart) \
  X(TLabel, LabelParkingArrivee) \
  X(TLabel, LabelTourSortie) \
  X(TLabel, LabelDistance) \
  X(TLabel, LabelParcours) \
  X(TLabel, LabelDirection) \
  X(TLabel, LabelDistanceMin) \
  X(TLabel, LabelTours) \
  X(TLabel, LabelParcoursEffectue) \
  X(TLabel, LabelDetour) \
  X(TLabel, Label12) \
  X(TLabel, LabelAttente) \
  X(TLabel, LabelVitesse) \
  X(TLabel, Label13) \
  X(TLabel, LabelNivPriorite) \
  X(TRadioButton, RadioButtonVehiculeNumero) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmInfosVehicule(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
