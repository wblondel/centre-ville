//---------------------------------------------------------------------------
// f_inftaxi_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_inftaxi.dfm : ne pas modifier
//
// COMPOSANTS_frmInfosTaxi(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmInfosTaxi(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_inftaxi_composantsH
#define f_inftaxi_composantsH

#define COMPOSANTS_frmInfosTaxi(X) \
  X(TLabel, Label0) \
  X(TLabel, LabelCoordonnees) \
  X(TButton, ButtonOk) \
  X(TRadioButton, RadioButtonAucunTaxi) \
  X(TGroupBox, GroupBoxTaxi) \
  X(TLabel, Label2) \
  X(TLabel, Label4) \
  X(TLabel, Label6) \
  X(TLabel, LabelTeteFileOrigine) \
  X(TLabel, LabelParcours) \
  X(TLabel, LabelDirection) \
  X(TLabel, Label9) \
  X(TLabel, LabelVitesse) \
  X(TLabel, Label3) \
  X(TLabel, LabelNumeroFile) \
  X(TLabel, Label5) \
  X(TLabel, LabelNbPersonnes) \
  X(TLabel, Label8) \
  X(TLabel, LabelParcoursDepuisDepart) \
  X(TLabel, Label7) \
  X(TLabel, LabelToursDepuisDepart) \
  X(TLabel, Label1) \
  X(TLabel, LabelSituation) \
  X(TLabel, Label10) \
  X(TLabel, LabelToursDepuisDepot) \
  X(TLabel, Label11) \
  X(TLabel, LabelToursDepuisRetour) \
  X(TLabel, Label12) \
  X(TLabel, LabelDistanceTeteFileDepart) \
  X(TRadioButton, RadioButtonTaxiNumero) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmInfosTaxi(X) \
  X(this, OnHide, FormHide) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
