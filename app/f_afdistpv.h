//---------------------------------------------------------------------------
#ifndef f_afdistpvH
#define f_afdistpvH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h"      // portage
// #include <Classes.hpp> // portage
// #include <Controls.hpp> // portage
// #include <Grids.hpp> // portage
// #include <ExtCtrls.hpp> // portage
// #include <StdCtrls.hpp> // portage
//---------------------------------------------------------------------------
class TfrmAffDistPlacesVehlib : public TForm
{
public:
    TStringGrid *StringGridDistances{}; // portage : __published	// Composants gérés par l'EDI
    void FormShow(TObject *Sender);
    static void FormHide(TObject *Sender);
    void FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);

private: // Déclarations de l'utilisateur
public:  // Déclarations de l'utilisateur
    explicit TfrmAffDistPlacesVehlib(TComponent *Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void EffaceInfos();
    void RafraichitInfos();
};
//---------------------------------------------------------------------------
extern TfrmAffDistPlacesVehlib *frmAffDistPlacesVehlib;
//---------------------------------------------------------------------------
#endif
