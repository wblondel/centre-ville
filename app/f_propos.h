//---------------------------------------------------------------------------
#ifndef f_proposH
#define f_proposH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Buttons.hpp> // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <ExtCtrls.hpp> // portage
//#include <Graphics.hpp> // portage
//#include <ImgList.hpp> // portage
//#include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
#include "base.h"
//#include <Vcl.Imaging.pngimage.hpp> // portage
//#include <Vcl.BaseImageCollection.hpp> // portage
//#include <Vcl.ImageCollection.hpp> // portage
//#include <Vcl.VirtualImageList.hpp> // portage
//#include <System.ImageList.hpp> // portage
//---------------------------------------------------------------------------
class TfrmAPropos : public TForm
{
public: // portage : __published
  TTimer *Timer;
  TLabel *LabelCV1;
  TLabel *LabelAuteur1;
  TLabel *LabelEMail;
  TLabel *LabelURL;
  TLabel *LabelTexte;
  TSpeedButton *SpeedButtonOk;
  TLabel *LabelCV2;
  TLabel *LabelAuteur2;
  TLabel *LabelSousVersion;
  TLabel *LabelGPL1;
  TLabel *LabelGPL2;
  TImage *ImageVersion;
  TImageCollection *ImageCollection;
  TVirtualImageList *VirtualImageList;
	TLabel *LabelAlignement;
	TImage *ImageEditionSpeciale;
	TLabel *LabelHDPI;
  void TimerTimer(TObject *Sender);
  void FormShow(TObject *Sender);
  void FormHide(TObject *Sender);
  void LabelURLClick(TObject *Sender);
  void LabelEMailClick(TObject *Sender);
  void FormCreate(TObject *Sender);
  void FormKeyPress(TObject *Sender, char &Key);
  void LabelClick(TObject *Sender);
private:
    int TempsEcoule;
    TBITMAP *Bitmap;
public:		// Déclarations de l'utilisateur
    bool FermetureAutomatique;
    TfrmAPropos(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmAPropos *frmAPropos;
//---------------------------------------------------------------------------
#endif
