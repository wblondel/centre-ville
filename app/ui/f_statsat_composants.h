//---------------------------------------------------------------------------
// f_statsat_composants.h - GÉNÉRÉ par tools/dfm2ui.py depuis src/f_statsat.dfm : ne pas modifier
//
// COMPOSANTS_frmStatsAttenteTrafic(X) : X(classe VCL, nom) pour chaque composant de la fiche,
// dans l'ordre du .dfm (membres à lier aux objets créés par uic).
// EVENEMENTS_frmStatsAttenteTrafic(X) : X(composant, événement, gestionnaire) pour chaque
// gestionnaire d'événement à connecter (« this » = la fiche).
//---------------------------------------------------------------------------
#ifndef f_statsat_composantsH
#define f_statsat_composantsH

#define COMPOSANTS_frmStatsAttenteTrafic(X) \
  X(TLabel, Label1) \
  X(TLabel, Label2) \
  X(TLabel, Label3) \
  X(TLabel, Label4) \
  X(TLabel, Label5) \
  X(TLabel, Label6) \
  X(TLabel, Label7) \
  X(TLabel, LabelAttenteCumulee) \
  X(TLabel, LabelAttenteMoyenne) \
  X(TLabel, LabelAttenteMaximale) \
  X(TLabel, LabelTraficCumule) \
  X(TLabel, LabelTraficMoyen) \
  X(TLabel, LabelTraficMaximal) \
  X(TLabel, Label8) \
  X(TRadioGroup, RadioGroupType) \
  X(TGroupBox, GroupBoxParkings) \
  X(TCSpinEdit, CSpinEditParkingArrivee) \
  X(TCheckBox, CheckBoxParkingArrivee) \
  X(TCheckBox, CheckBoxParkingDepart) \
  X(TCSpinEdit, CSpinEditParkingDepart) \
  X(TGroupBox, GroupBoxLegende) \
  X(TLabel, LabelMax) \
  X(TLabel, Label10) \
  X(TPaintBox, PaintBox) \
  X(TBitBtn, BitBtn21) \
  X(TBitBtn, BitBtn31) \
  X(TBitBtn, BitBtn11) \
  X(TBitBtn, BitBtn13) \
  X(TBitBtn, BitBtn23) \
  X(TBitBtn, BitBtn33) \
  X(TBitBtn, BitBtn12) \
  X(TBitBtn, BitBtn32) \
  X(TCheckBox, CheckBoxCoins) \
  X(TPopupMenu, PopupMenu) \
  X(TMenuItem, RubriqueHautGauche) \
  X(TMenuItem, RubriqueHaut) \
  X(TMenuItem, RubriqueHautDroite) \
  X(TMenuItem, Sep1) \
  X(TMenuItem, RubriqueGauche) \
  X(TMenuItem, RubriqueDroite) \
  X(TMenuItem, RubriqueZoom) \
  X(TMenuItem, Sep2) \
  X(TMenuItem, RubriqueBasGauche) \
  X(TMenuItem, RubriqueBas) \
  X(TMenuItem, RubriqueBasDroite) \

#define EVENEMENTS_frmStatsAttenteTrafic(X) \
  X(this, OnKeyDown, FormKeyDown) \
  X(this, OnShow, FormShow) \
  X(RadioGroupType, OnClick, RadioGroupTypeClick) \
  X(CSpinEditParkingArrivee, OnChange, ParametresParkingsClick) \
  X(CheckBoxParkingArrivee, OnClick, ParametresParkingsClick) \
  X(CheckBoxParkingDepart, OnClick, ParametresParkingsClick) \
  X(CSpinEditParkingDepart, OnChange, ParametresParkingsClick) \
  X(PaintBox, OnPaint, PaintBoxPaint) \
  X(BitBtn21, OnClick, BitBtnDirClick) \
  X(BitBtn31, OnClick, BitBtnDirClick) \
  X(BitBtn11, OnClick, BitBtnDirClick) \
  X(BitBtn13, OnClick, BitBtnDirClick) \
  X(BitBtn23, OnClick, BitBtnDirClick) \
  X(BitBtn33, OnClick, BitBtnDirClick) \
  X(BitBtn12, OnClick, BitBtnDirClick) \
  X(BitBtn32, OnClick, BitBtnDirClick) \
  X(RubriqueHautGauche, OnClick, RubriqueClick) \
  X(RubriqueHaut, OnClick, RubriqueClick) \
  X(RubriqueHautDroite, OnClick, RubriqueClick) \
  X(RubriqueGauche, OnClick, RubriqueClick) \
  X(RubriqueDroite, OnClick, RubriqueClick) \
  X(RubriqueZoom, OnClick, RubriqueClick) \
  X(RubriqueBasGauche, OnClick, RubriqueClick) \
  X(RubriqueBas, OnClick, RubriqueClick) \
  X(RubriqueBasDroite, OnClick, RubriqueClick) \

#endif
