//---------------------------------------------------------------------------
#ifndef f_carfoursH
#define f_carfoursH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Buttons.hpp> // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <StdCtrls.hpp> // portage
//#include <System.ImageList.hpp> // portage
//#include <Vcl.BaseImageCollection.hpp> // portage
//#include <Vcl.ImageCollection.hpp> // portage
//#include <Vcl.ImgList.hpp> // portage
//#include <Vcl.VirtualImageList.hpp> // portage
//---------------------------------------------------------------------------
class TfrmCarrefours : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TGroupBox *GroupBox34;
    TSpeedButton *SpeedButton34b1;
    TSpeedButton *SpeedButton34b2;
    TSpeedButton *SpeedButton34b3;
    TSpeedButton *SpeedButton34a3;
    TSpeedButton *SpeedButton34a2;
    TSpeedButton *SpeedButton34a1;
    TGroupBox *GroupBox43;
    TSpeedButton *SpeedButton43b1;
    TSpeedButton *SpeedButton43b2;
    TSpeedButton *SpeedButton43b3;
    TSpeedButton *SpeedButton43a3;
    TSpeedButton *SpeedButton43a2;
    TSpeedButton *SpeedButton43a1;
    TGroupBox *GroupBox44;
    TSpeedButton *SpeedButton44b;
    TSpeedButton *SpeedButton44c;
    TSpeedButton *SpeedButton44a;
    TGroupBox *GroupBox66;
    TSpeedButton *SpeedButton66a;
    TSpeedButton *SpeedButton66b;
    TGroupBox *GroupBox88;
    TSpeedButton *SpeedButton88;
    TButton *ButtonAnnuler;
    TLabel *Label;
    TButton *ButtonAide;
	TImageCollection *ImageCollection;
	TVirtualImageList *VirtualImageList64;
	TVirtualImageList *VirtualImageList96;
	TVirtualImageList *VirtualImageList128;
    void SpeedButtonClick(TObject *Sender);
    void ButtonAideClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    
private:	// Déclarations de l'utilisateur
	void AffecteGlyphe(TVirtualImageList *VirtualImageList, const int NumGlyphe, TBitmap *Glyphe)
	 { // v5.4 pour affecter les SpeedButton->Glyph à partir des virtualImageList et supporter le HDPI
	  VirtualImageList->GetBitmap(NumGlyphe, Glyphe);
	 };
public:		// Déclarations de l'utilisateur
    TfrmCarrefours(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void Active(int Colonnes, int Lignes);
};
//---------------------------------------------------------------------------
extern TfrmCarrefours *frmCarrefours;
extern int Carrefours(int Colonnes, int Lignes);
//---------------------------------------------------------------------------
#endif
