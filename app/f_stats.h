//---------------------------------------------------------------------------
#ifndef f_statsH
#define f_statsH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h"      // portage
// #include <Classes.hpp> // portage
// #include <Controls.hpp> // portage
// #include <ExtCtrls.hpp> // portage
//---------------------------------------------------------------------------
class TfrmStatistiques : public TForm
{
public:
    TPaintBox *PaintBoxStats{}; // portage : __published	// Composants gérés par l'EDI
    void PaintBoxStatsPaint(TObject *Sender);
    static void FormShow(TObject *Sender);
    static void FormHide(TObject *Sender);
    void FormCreate(TObject *Sender);
    void FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);

private: // Déclarations de l'utilisateur
public:
    bool AffichageEpidemique{false}; // Déclarations de l'utilisateur // v5.4.1. Définit avant l'appel à FormShow par
                                     // frmSimulation
    explicit TfrmStatistiques(TComponent *Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmStatistiques *frmStatistiques;
//---------------------------------------------------------------------------
#endif
