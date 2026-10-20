//---------------------------------------------------------------------------
// f_prmvehlib_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_prmvehlib.dfm : ne pas modifier
//
// COMPOSANTS_frmParamVehlib(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmParamVehlib(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_prmvehlib_composantsH
#define f_prmvehlib_composantsH

#define COMPOSANTS_frmParamVehlib(X) \
  X(TLabel, Label21) \
  X(TLabel, Label22) \
  X(TLabel, Label3) \
  X(TCSpinEdit, CSpinEditNbMaxPersonnesVehlib) \
  X(TGroupBox, GroupBox) \
  X(TLabel, LabelNbCasesMaxPsgPtn) \
  X(TLabel, Label23) \
  X(TLabel, LabelProbaVehlib) \
  X(TLabel, Label25) \
  X(TLabel, Label1) \
  X(TLabel, Label2) \
  X(TCSpinEdit, CSpinEditNbCasesMaxPlaceVehlib) \
  X(TCSpinEdit, CSpinEditProbaDirPlaceVehlib) \
  X(TCSpinEdit, CSpinEditTauxRemplissage) \
  X(TButton, ButtonOk) \
  X(TButton, ButtonAnnuler) \
  X(TButton, ButtonAppliquer) \
  X(TButton, ButtonAide) \

#define EVENEMENTS_frmParamVehlib(X) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(CSpinEditNbMaxPersonnesVehlib, OnChange, CSpinEditParametresChange) \
  X(CSpinEditNbCasesMaxPlaceVehlib, OnChange, CSpinEditParametresChange) \
  X(CSpinEditProbaDirPlaceVehlib, OnChange, CSpinEditParametresChange) \
  X(CSpinEditTauxRemplissage, OnChange, CSpinEditParametresChange) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAppliquer, OnClick, ButtonAppliquerClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \

#endif
