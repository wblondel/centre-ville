//---------------------------------------------------------------------------
// f_prmtaxi_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_prmtaxi.dfm : ne pas modifier
//
// COMPOSANTS_frmParamTaxi(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmParamTaxi(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_prmtaxi_composantsH
#define f_prmtaxi_composantsH

#define COMPOSANTS_frmParamTaxi(X) \
  X(TLabel, Label21) \
  X(TLabel, Label22) \
  X(TGroupBox, GroupBox) \
  X(TLabel, LabelNbCasesMaxPsgPtn) \
  X(TLabel, Label23) \
  X(TLabel, Label24) \
  X(TLabel, Label17) \
  X(TLabel, LabelProbaPsgPt) \
  X(TLabel, Label25) \
  X(TLabel, Label26) \
  X(TLabel, Label27) \
  X(TCSpinEdit, CSpinEditNbCasesMaxPlaceTaxi) \
  X(TCSpinEdit, CSpinEditNbMaxToursAttente) \
  X(TCSpinEdit, CSpinEditProbaDirPlaceTaxi) \
  X(TCSpinEdit, CSpinEditProbaArretDmd) \
  X(TButton, ButtonOk) \
  X(TButton, ButtonAnnuler) \
  X(TButton, ButtonAppliquer) \
  X(TButton, ButtonAide) \
  X(TCSpinEdit, CSpinEditNbMaxPersonnesTaxi) \

#define EVENEMENTS_frmParamTaxi(X) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(CSpinEditNbCasesMaxPlaceTaxi, OnChange, CSpinEditParametresChange) \
  X(CSpinEditNbMaxToursAttente, OnChange, CSpinEditParametresChange) \
  X(CSpinEditProbaDirPlaceTaxi, OnChange, CSpinEditParametresChange) \
  X(CSpinEditProbaArretDmd, OnChange, CSpinEditParametresChange) \
  X(ButtonOk, OnClick, ButtonOkClick) \
  X(ButtonAppliquer, OnClick, ButtonAppliquerClick) \
  X(ButtonAide, OnClick, ButtonAideClick) \
  X(CSpinEditNbMaxPersonnesTaxi, OnChange, CSpinEditParametresChange) \

#endif
