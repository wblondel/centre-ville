//---------------------------------------------------------------------------
#include "compat/chaine.h"
#include <array>
#include "simul.h"
#include "vcl.h"
#include "vcl/fiche.h" // portage : remplace <vcl.h>
// #pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "f_inftram.h"
#include "f_simul.h"
#include "base.h"
//---------------------------------------------------------------------------
#include "ui_f_inftram.h"         // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_inftram_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
// Pour asFrmNumeroCoordonnees
constexpr int NXY = 0;
constexpr int DRN = 1;
constexpr int SVT = 2;
//---------------------------------------------------------------------------
TfrmInfosTram *frmInfosTram;
//---------------------------------------------------------------------------
constexpr const char *asDebutTitreCadreTram = "                           ";
constexpr std::array<const char *, 3> asFrmNumeroCoordonnees = {
    "%d (%2.2d, %2.2d)",
    "%d (%2.2d, %2.2d)  jusqu'au tour %d",
    "%d (%2.2d, %2.2d) depuis le tour %d",
};
constexpr const char *asFrmCoordonnees = "%2.2d, %2.2d";
constexpr const char *asFrmPourcentage = "0.000%"; // v3.0. v5.4.1 : FormatFloat n'utilise pas le format "%.3f%%" mais
                                                   // // "#.000%"
constexpr const char *asFrmVitesse = "%d/%d";
//---------------------------------------------------------------------------
TfrmInfosTram::TfrmInfosTram(TComponent *Owner) : TForm(Owner)
{
    ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
}
//---------------------------------------------------------------------------
void TfrmInfosTram::RendreVisibleInfos(bool ARendreVisible)
{
    int i = 0;
    GroupBoxTram->Caption = asDebutTitreCadreTram;
    if (ARendreVisible)
    {
        RadioButtonTramNumero->Checked = true;
        GroupBoxTram->Caption = GroupBoxTram->Caption +
                                Format(asFrmNumeroCoordonnees[NXY], TableauDeConstantes(v->NumTram, Tram->x, Tram->y)) +
                                " ";
    }
    else
    {
        RadioButtonAucunTram->Checked = true;
    }
    RadioButtonTramNumero->Enabled = ARendreVisible;
    RadioButtonAucunTram->Enabled = !ARendreVisible;
    for (i = 1; i < 16; i++)
    {
        (dynamic_cast<TLabel *>(FindComponent("Label" + IntToStr(i))))->Enabled = ARendreVisible;
    }
    LabelDirection->Visible = ARendreVisible;
    LabelNumeroLigne->Visible = ARendreVisible;
    LabelNbPersonnes->Visible = ARendreVisible;
    LabelDernierArret->Visible = ARendreVisible;
    LabelProchainArret->Visible = ARendreVisible;
    LabelTours->Visible = ARendreVisible;
    LabelParcours->Visible = ARendreVisible;
    LabelToursEntreArrets->Visible = ARendreVisible;
    LabelParcoursEntreArrets->Visible = ARendreVisible;
    LabelDistance->Visible = ARendreVisible;
    LabelDistanceMin->Visible = ARendreVisible;
    LabelParcoursEffectue->Visible = ARendreVisible;
    LabelDetour->Visible = ARendreVisible;
    LabelAttente->Visible = ARendreVisible;
    LabelVitesse->Visible = ARendreVisible;
}
//---------------------------------------------------------------------------
void TfrmInfosTram::RafraichitInfos()
{
    int i = 0;
    int NbTours = 0;
    int NumDrnArret = 0;
    int NumArretSvt = 0;
    int NbDirPoss = 0;
    int DejaParcouru = 0;
    int ResteAParcourir = 0;
    int DistMin = 0;
    voie *DrnArret = nullptr;
    bool Afficher = false;
    x = frmSimulation->DrawGridSimul->Col;
    y = frmSimulation->DrawGridSimul->Row;
    v = &cv->v[x][y];
    Tram = nullptr;
    LabelCoordonnees->Caption = Format(asFrmCoordonnees, TableauDeConstantes(x, y));
    Afficher = (v->NumTram > 0);
    if (Afficher)
    {
        Tram = &cv->Tram[v->NumTram - 1];
        LabelDirection->Caption = asDir[Tram->Dir];
        LabelNumeroLigne->Caption = AnsiString(static_cast<char>('A' + Tram->NumLigne - 1));
        NumDrnArret = Tram->NumDrnArret;
        if (NumDrnArret != 0)
        {
            LabelDernierArret->Caption =
                Format(asFrmNumeroCoordonnees[DRN],
                       TableauDeConstantes(NumDrnArret, cv->ArretTram[NumDrnArret - 1].x,
                                           cv->ArretTram[NumDrnArret - 1].y, Tram->TourArriveDrnArret));
            DrnArret = &cv->v[cv->ArretTram[NumDrnArret - 1].x][cv->ArretTram[NumDrnArret - 1].y];
        }
        else
        { // Si pas encore d'arrêt... On prend le premier arrêt de la ligne comme dernier arrêt
            LabelDernierArret->Caption =
                Format(asFrmNumeroCoordonnees[DRN], TableauDeConstantes(0, cv->ArretTram[0].x, cv->ArretTram[0].y, 0));
            DrnArret = &cv->v[cv->ArretTram[0].x][cv->ArretTram[0].y];
        }
        // portage : l'ordre d'évaluation des arguments n'est pas spécifié en C++ ; le binaire
        // d'origine les évalue de gauche à droite (engine/README.md) : NumArretSvt d'abord
        NumArretSvt = Tram->NumArretSvt;
        LabelProchainArret->Caption =
            Format(asFrmNumeroCoordonnees[SVT],
                   TableauDeConstantes(NumArretSvt, cv->ArretTram[NumArretSvt - 1].x, cv->ArretTram[NumArretSvt - 1].y,
                                       Tram->TourDepartArretSvt)); // v6.0.2 : de ce tram (l'original lisait le
                                                                   // premier)
        LabelNbPersonnes->Caption = IntToStr(Tram->NbPersonnes);
        LabelTours->Caption = IntToStr(cv->TourCrt); // v6.0.2 : correct, tous les trams partent au tour 0
                                                     // (PrepareTrams) : c'est TourCrt-TourDepart des bus
        LabelParcours->Caption = IntToStr(Tram->NbCasesParcourues);
        LabelToursEntreArrets->Caption = IntToStr(NbTours = cv->TourCrt - Tram->TourArriveDrnArret);
        LabelParcoursEntreArrets->Caption = IntToStr(DejaParcouru = Tram->NbCasesParcouruesDepuisDrnArret);
        LabelDistance->Caption = IntToStr(
            ResteAParcourir = v->DistArretTram()[NumArretSvt - 1][v->NumDirPossSensVoieTram(indefinie, Tram->Dir)]);
        NbDirPoss = DrnArret->NbDirPossSensVoieTram(indefinie);
        for (i = 0, DistMin = DIST_INDEFINIE; i < NbDirPoss; i++)
        {
            DistMin = min(DistMin, DrnArret->DistArretTram()[NumArretSvt - 1][i]);
        }
        LabelDistanceMin->Caption = IntToStr(DistMin);
        // v1.8.4 : FormatFloat au lieu de Format
        LabelParcoursEffectue->Caption =
            FormatFloat(asFrmPourcentage, (100.0 * DejaParcouru) / (DejaParcouru + ResteAParcourir));
        LabelDetour->Caption =
            FormatFloat(asFrmPourcentage, ((100.0 * (DejaParcouru + ResteAParcourir)) / DistMin) - 100.0);
        if (NbTours != 0)
        {
            LabelAttente->Caption = FormatFloat(asFrmPourcentage, (100.0 * (NbTours - DejaParcouru)) / NbTours);
        }
        else
        {
            LabelAttente->Caption = "0,000%";
        }
        LabelVitesse->Caption = Format(asFrmVitesse, TableauDeConstantes(1, cv->NbVitesses));
    }
    RendreVisibleInfos(Afficher);
}
//---------------------------------------------------------------------------
void TfrmInfosTram::FormShow(TObject * /*Sender*/)
{
    frmSimulation->ActionDetailTram->Checked = true;
    RafraichitInfos();
}
//---------------------------------------------------------------------------
void TfrmInfosTram::FormHide(TObject * /*Sender*/)
{
    frmSimulation->ActionDetailTram->Checked = false;
}
//---------------------------------------------------------------------------
void TfrmInfosTram::ButtonOkClick(TObject * /*Sender*/)
{
    Hide();
}
//---------------------------------------------------------------------------
void TfrmInfosTram::ButtonAideClick(TObject * /*Sender*/)
{
    AfficheAideContextuelle(this); // v3.8
}
//---------------------------------------------------------------------------

void TfrmInfosTram::FormKeyDown(TObject * /*Sender*/, WORD &Key, TShiftState /*Shift*/)
{ // v5.3 (nécessite TForm:KeyPreview à true)
    if (Key == 112)
    {
        AfficheAideContextuelle(this);
    }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
// NOLINTBEGIN(bugprone-macro-parentheses) : noms du composant, de l'événement et du gestionnaire
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire)                                                         \
    Connecte(Composant->Evenement, this, &TfrmInfosTram::Gestionnaire);
// NOLINTEND(bugprone-macro-parentheses)
void TfrmInfosTram::ChargeFiche()
{
    Ui::frmInfosTram().setupUi(this);
    LieurComposants Lieur(this, "f_inftram", Composants);
    COMPOSANTS_frmInfosTram(LIE_COMPOSANT) FinChargement(Lieur);
    EVENEMENTS_frmInfosTram(CONNECTE_EVENEMENT) if (OnCreate)
    {
        OnCreate(this);
    }
}
//---------------------------------------------------------------------------
