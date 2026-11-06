//---------------------------------------------------------------------------
#ifndef f_genresH
#define f_genresH
//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace les en-têtes de la VCL
#include "base.h"      // portage
// #include <Classes.hpp> // portage
// #include <Controls.hpp> // portage
// #include <StdCtrls.hpp> // portage
// #include <Forms.hpp> // portage
// #include <ComCtrls.hpp> // portage
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TfrmGeneration : public TForm
{
public:
    TGroupBox *GroupBox{}; // portage : __published	// Composants gérés par l'EDI
    TTrackBar *TrackBarTailleMinSegments{};
    TLabel *Label1{};
    TLabel *Label2{};
    TTrackBar *TrackBarProbaSortieIntersection{};
    TTrackBar *TrackBarTailleMaxSegments{};
    TLabel *Label3{};
    TLabel *Label4{};
    TButton *ButtonAnnuler{};
    TButton *ButtonOk{};
    TLabel *Label5{};
    TTrackBar *TrackBarProbaEntreeIntersection{};
    TLabel *Label6{};
    TTrackBar *TrackBarProbaTraverseeRoute{};
    TLabel *LabelRq{};
    TButton *ButtonAide{};
    void ButtonOkClick(TObject *Sender);
    void ButtonAideClick(TObject *Sender);
    void FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);

private: // Déclarations de l'utilisateur
public:  // Déclarations de l'utilisateur
    explicit TfrmGeneration(TComponent *Owner);
    void ChargeFiche(); // portage : chargement du .dfm (fiche générée par dfm2ui)
};
//---------------------------------------------------------------------------
extern TfrmGeneration *frmGeneration; // v6.0.17 : déclaré (en commentaire dans l'original)
extern void GenereReseau();
//---------------------------------------------------------------------------
#endif
