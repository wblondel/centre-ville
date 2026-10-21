//---------------------------------------------------------------------------
#ifndef f_defvpH
#define f_defvpH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Buttons.hpp> // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <ExtCtrls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//#include <Graphics.hpp> // portage
//---------------------------------------------------------------------------
#include "simul.h"
//---------------------------------------------------------------------------
class TfrmDefVoiePietons : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
  TLabel *Label1;
  TLabel *LabelCoordonnees;
  TButton *ButtonOk;
  TButton *ButtonAnnuler;
  TButton *ButtonAppliquer;
  TButton *ButtonAide;
  TImage *ImageEE;
  TImage *ImageOO;
  TImage *ImageSS;
  TImage *ImageNN;
  TImage *ImageNE;
  TImage *ImageSE;
  TImage *ImageSO;
  TImage *ImageNO;
  TImage *ImageS;
  TImage *ImageO;
  TImage *ImageN;
  TImage *ImageE;
  TImage *Image0;
  TSpeedButton *SpeedButtonN;
  TSpeedButton *SpeedButtonE;
  TSpeedButton *SpeedButtonO;
  TSpeedButton *SpeedButtonS;
  TLabel *Label2;
  TImage *ImageAttention;
    void FormShow(TObject *Sender);
    void ButtonOkClick(TObject *Sender);
    void ButtonAppliquerClick(TObject *Sender);
    void ButtonAnnulerClick(TObject *Sender);
    void FormHide(TObject *Sender);
    void ButtonAideClick(TObject *Sender);
  void FormCreate(TObject *Sender);
  void SpeedButtonClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
    bool FeuExistant[NBDIR]; // Indique lors de l'affichage de la fenêtre, l'existence des feux. Sert à l'annulation (suppression)
    voie *v;
    int x, y, xfo[NBDIR], yfo[NBDIR];
    TSpeedButton *SpeedButton[NBDIR];
    TImage *Image[NBDIR];
    void DefinitFeu(direction Dir);
public:		// Déclarations de l'utilisateur
    TfrmDefVoiePietons(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void RafraichitInfos();
    void AppliqueChoix();
};
//---------------------------------------------------------------------------
extern TfrmDefVoiePietons *frmDefVoiePietons;
//---------------------------------------------------------------------------
#endif
