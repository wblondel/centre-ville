//---------------------------------------------------------------------------
#ifndef f_statsvH
#define f_statsvH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <ComCtrls.hpp> // portage
//#include <Controls.hpp> // portage
//#include <Grids.hpp> // portage
//#include <StdCtrls.hpp> // portage
//#include <Classes.hpp> // portage
//---------------------------------------------------------------------------
class TfrmStatsVehicules : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TButton *ButtonFermer;
    TTabControl *TabControl;
    TStringGrid *StringGridStatsVehicules;
    TStringGrid *StringGridCoordParkings;
    TButton *ButtonAide;
    void FormShow(TObject *Sender);
    void FormHide(TObject *Sender);
    void ButtonFermerClick(TObject *Sender);
    void TabControlChange(TObject *Sender);
    void ButtonAideClick(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
public:		// Déclarations de l'utilisateur
    TfrmStatsVehicules(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void RafraichitInfos();
};
//---------------------------------------------------------------------------
extern TfrmStatsVehicules *frmStatsVehicules;
//---------------------------------------------------------------------------
#endif
