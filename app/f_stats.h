//---------------------------------------------------------------------------
#ifndef f_statsH
#define f_statsH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <ExtCtrls.hpp> // portage
//---------------------------------------------------------------------------
class TfrmStatistiques : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TPaintBox *PaintBoxStats;
    void PaintBoxStatsPaint(TObject *Sender);
    void FormShow(TObject *Sender);
    void FormHide(TObject *Sender);
    void FormCreate(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Déclarations de l'utilisateur
public:		// Déclarations de l'utilisateur
    bool AffichageEpidemique; // v5.4.1. Définit avant l'appel à FormShow par frmSimulation
    TfrmStatistiques(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmStatistiques *frmStatistiques;
//---------------------------------------------------------------------------
#endif
