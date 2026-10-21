//---------------------------------------------------------------------------
#ifndef f_afdistftH
#define f_afdistftH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h" // portage
//#include <Classes.hpp> // portage
//#include <Controls.hpp> // portage
//#include <Grids.hpp> // portage
//---------------------------------------------------------------------------
class TfrmAffDistFilesTaxi : public TForm
{
public: // portage : __published	// Composants gérés par l'EDI
    TStringGrid *StringGridDistances;
    void FormShow(TObject *Sender);
    void FormHide(TObject *Sender);
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);


private:	// Déclarations de l'utilisateur
public:		// Déclarations de l'utilisateur
    TfrmAffDistFilesTaxi(TComponent* Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
    void EffaceInfos();
    void RafraichitInfos();
};
//---------------------------------------------------------------------------
extern TfrmAffDistFilesTaxi *frmAffDistFilesTaxi;
//---------------------------------------------------------------------------
#endif
