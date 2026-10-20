//---------------------------------------------------------------------------
// f_inftram_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_inftram.dfm : ne pas modifier
//
// COMPOSANTS_frmInfosTram(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmInfosTram(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_inftram_composantsH
#define f_inftram_composantsH

#define COMPOSANTS_frmInfosTram(X) \
  X(TLabel, Label0) \
  X(TLabel, LabelCoordonnees) \
  X(TLabel, Label17) \
  X(TLabel, Label18) \
  X(TButton, ButtonOk) \
  X(TRadioButton, RadioButtonAucunTram) \
  X(TGroupBox, GroupBoxTram) \
  X(TLabel, Label1) \
  X(TLabel, Label3) \
  X(TLabel, Label5) \
  X(TLabel, Label6) \
  X(TLabel, Label9) \
  X(TLabel, Label10) \
  X(TLabel, Label11) \
  X(TLabel, LabelDernierArret) \
  X(TLabel, LabelParcours) \
  X(TLabel, LabelDirection) \
  X(TLabel, LabelTours) \
  X(TLabel, LabelParcoursEffectue) \
  X(TLabel, LabelDetour) \
  X(TLabel, Label12) \
  X(TLabel, LabelAttente) \
  X(TLabel, LabelVitesse) \
  X(TLabel, Label2) \
  X(TLabel, LabelNumeroLigne) \
  X(TLabel, Label7) \
  X(TLabel, LabelDistance) \
  X(TLabel, LabelDistanceMin) \
  X(TLabel, Label8) \
  X(TLabel, Label4) \
  X(TLabel, LabelProchainArret) \
  X(TLabel, Label13) \
  X(TLabel, LabelNbPersonnes) \
  X(TLabel, Label14) \
  X(TLabel, LabelParcoursEntreArrets) \
  X(TLabel, Label15) \
  X(TLabel, LabelToursEntreArrets) \
  X(TRadioButton, RadioButtonTramNumero) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmInfosTram(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
