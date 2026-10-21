//---------------------------------------------------------------------------
#ifndef f_statsatH
#define f_statsatH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include "CSPIN.h" // portage
//#include <Buttons.hpp> // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <ExtCtrls.hpp> // portage
//#include <Menus.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "base.h"
//---------------------------------------------------------------------------
class TfrmStatsAttenteTrafic : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TRadioGroup *RadioGroupType;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TLabel *Label4;
    TLabel *Label5;
    TLabel *Label6;
    TLabel *Label7;
    TLabel *LabelAttenteCumulee;
    TLabel *LabelAttenteMoyenne;
    TLabel *LabelAttenteMaximale;
    TLabel *LabelTraficCumule;
    TLabel *LabelTraficMoyen;
    TLabel *LabelTraficMaximal;
    TLabel *Label8;
  TGroupBox *GroupBoxParkings;
  TCSpinEdit *CSpinEditParkingArrivee;
  TCheckBox *CheckBoxParkingArrivee;
  TCheckBox *CheckBoxParkingDepart;
  TCSpinEdit *CSpinEditParkingDepart;
    TGroupBox *GroupBoxLegende;
    TLabel *LabelMax;
    TLabel *Label10;
    TPaintBox *PaintBox;
    TBitBtn *BitBtn21;
    TBitBtn *BitBtn31;
    TBitBtn *BitBtn11;
    TBitBtn *BitBtn13;
    TBitBtn *BitBtn23;
    TBitBtn *BitBtn33;
    TBitBtn *BitBtn12;
    TBitBtn *BitBtn32;
    TCheckBox *CheckBoxCoins;
    TPopupMenu *PopupMenu;
    TMenuItem *RubriqueHautGauche;
    TMenuItem *RubriqueHaut;
    TMenuItem *RubriqueHautDroite;
    TMenuItem *RubriqueGauche;
    TMenuItem *RubriqueDroite;
    TMenuItem *RubriqueBasGauche;
    TMenuItem *RubriqueBas;
    TMenuItem *RubriqueBasDroite;
    TMenuItem *Sep1;
    TMenuItem *Sep2;
    TMenuItem *RubriqueZoom;
    void PaintBoxPaint(TObject *Sender);
    void FormShow(TObject *Sender);

    void ParametresParkingsClick(TObject *Sender);
    void RadioGroupTypeClick(TObject *Sender);
    void BitBtnDirClick(TObject *Sender);

    void RubriqueClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
    int m[2];
    void RecalculeEtVisualise();

public:		// Déclarations de l'utilisateur
    TfrmStatsAttenteTrafic(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    TColor CouleurCase(int x, int y);
    AnsiString asMsgDetailCase(int x, int y);
};
//---------------------------------------------------------------------------
extern TfrmStatsAttenteTrafic *frmStatsAttenteTrafic;
//---------------------------------------------------------------------------
#endif
