//---------------------------------------------------------------------------
// f_infbus_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_infbus.dfm : ne pas modifier
//
// COMPOSANTS_frmInfosBus(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmInfosBus(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_infbus_composantsH
#define f_infbus_composantsH

#define COMPOSANTS_frmInfosBus(X) \
  X(TLabel, Label0) \
  X(TLabel, LabelCoordonnees) \
  X(TLabel, Label17) \
  X(TLabel, Label18) \
  X(TButton, ButtonOk) \
  X(TRadioButton, RadioButtonAucunBus) \
  X(TGroupBox, GroupBoxBus) \
  X(TLabel, Label1) \
  X(TLabel, Label3) \
  X(TLabel, Label5) \
  X(TLabel, Label6) \
  X(TLabel, Label7) \
  X(TLabel, Label10) \
  X(TLabel, Label11) \
  X(TLabel, Label12) \
  X(TLabel, LabelDernierArret) \
  X(TLabel, LabelTourSortie) \
  X(TLabel, LabelParcours) \
  X(TLabel, LabelDirection) \
  X(TLabel, LabelTours) \
  X(TLabel, LabelParcoursEffectue) \
  X(TLabel, LabelDetour) \
  X(TLabel, Label13) \
  X(TLabel, LabelAttente) \
  X(TLabel, LabelVitesse) \
  X(TLabel, Label2) \
  X(TLabel, LabelNumeroLigne) \
  X(TLabel, Label8) \
  X(TLabel, LabelDistance) \
  X(TLabel, LabelDistanceMin) \
  X(TLabel, Label9) \
  X(TLabel, Label4) \
  X(TLabel, LabelProchainArret) \
  X(TLabel, Label14) \
  X(TLabel, LabelNbPersonnes) \
  X(TLabel, Label15) \
  X(TLabel, LabelParcoursEntreArrets) \
  X(TLabel, Label16) \
  X(TLabel, LabelToursEntreArrets) \
  X(TRadioButton, RadioButtonBusNumero) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmInfosBus(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
