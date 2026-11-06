//---------------------------------------------------------------------------
#include "base.h"
#include <array>
#include "compat/chaine.h"
#include "simul.h"
#include "vcl.h"
#include "vcl/fiche.h" // portage : remplace <vcl.h>
// #pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "f_infptn.h"
#include "f_simul.h"
#include "f_rech.h"
//---------------------------------------------------------------------------
#include "ui_f_infptn.h"         // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_infptn_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
constexpr int NBSITUATIONS = 17; // v5.3 (+3) v5.4.1 (+1)
constexpr int NBPOSITIONS = 6;   // v5.3 (+1)
//---------------------------------------------------------------------------
// Positions
constexpr int DANSUNBUS = 2;
constexpr int DANSUNTRAM = 3;
constexpr int DANSUNTAXI = 4;
constexpr int DANSVEHLIB = 5; // v5.3
//---------------------------------------------------------------------------
// Situations
constexpr int PSGPTTRV = 1;
constexpr int ARRETBUSTRV = 2;
constexpr int ARRETTRAMTRV = 3;
constexpr int FILETAXITRV = 4;
constexpr int PLACEVEHLIBTRV = 5;
constexpr int ATTENDUNBUS = 6;
constexpr int ATTENDDSLEBUS = 7;
constexpr int ATTENDUNTRAM = 8;
constexpr int ATTENDDSLETRAM = 9;
constexpr int ATTENDUNTAXI = 10;
constexpr int ATTENDDSLETAXI = 11;
constexpr int ATTENDUNVEHLIB = 12; // v5.3
constexpr int ATTENDDSVEHLIB = 13; // v5.3
constexpr int VEUTDESCENDRE = 14;
constexpr int ECRASE = 15;
constexpr int MORT = 16; // v5.4.1
//---------------------------------------------------------------------------
TfrmInfosPieton *frmInfosPieton;
//---------------------------------------------------------------------------
constexpr const char *asLabel = "Label";
constexpr const char *asDebutTitreCadrePieton = "                           ";
constexpr const char *asFrmNumeroCoordonnees = "%d (%2.2d, %2.2d)";
constexpr const char *asFrmCoordonnees = "%2.2d, %2.2d";
constexpr const char *asFrmPourcents = "0.0%"; // v5.4.1 : porucentage à 0,1 près
constexpr const char *asFrmDemiCase = "0.0";   // v5.4.1 : FormatFloat n'utilise pas le format "%.1f" mais "#.0"
constexpr std::array<const char *, NBPOSITIONS> asPosition = {
    "Entrée de la case", "Sortie de la case", "Dans un bus",
    "Dans un tram",      "Dans un taxi",      "Dans un vehlib"}; // v5.3 (vehlib). v6.0.2 : « ans un vehlib »
constexpr std::array<const char *, NBSITUATIONS> asSituation = {
    "Se promène",
    "Se dirige vers un passage piéton",
    "Se dirige vers un arrêt de bus",
    "Se dirige vers un arrêt de trams",
    "Se dirige vers une file de taxis",
    "Se dirige vers une file de vehlibs",
    "Attend un bus à l'arrêt",
    "Attend dans le bus",
    "Attend un tram à l'arrêt",
    "Attend dans le tram",
    "Attend un taxi en tête de file",
    "Attend dans le taxi",
    "Attend un vehlib",
    "Attend dans le vehlib",
    "Descend au prochain arrêt",
    "Ecrasé (mode sanglant)",
    "Mort (épidémie)",
}; // v5.3 // v5.3 // v5.3
constexpr std::array<const char *, 2> asOuiNon = {"Non", "Oui"}; // v5.4.1
constexpr const char *asFrmPrmDrn = "%d, %d";                    // v5.4.1
constexpr std::array<const char *, 2> asEtatFinal = {"Guéri", "Mort"};
constexpr const char *asNA = "N/A";
//---------------------------------------------------------------------------
TfrmInfosPieton::TfrmInfosPieton(TComponent *Owner) : TForm(Owner)
{
    ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
    NumPieton = 0;
}
//---------------------------------------------------------------------------
void TfrmInfosPieton::RendreVisibleInfos(bool ARendreVisible)
{
    int i = 0;
    GroupBoxPieton->Caption = asDebutTitreCadrePieton;
    if (ARendreVisible)
    { // v5.4.1 : manquait tram, taxi et vehlib... v6.0.2 : coordonnées lues dans leur tableau (l'original lisait les
      // bus)
        if (p->NumBus != 0)
        {
            GroupBoxPieton->Caption =
                GroupBoxPieton->Caption +
                Format(asFrmNumeroCoordonnees,
                       TableauDeConstantes(NumPieton, cv->Bus[p->NumBus - 1].x, cv->Bus[p->NumBus - 1].y));
        }
        else if (p->NumTram != 0)
        {
            GroupBoxPieton->Caption =
                GroupBoxPieton->Caption +
                Format(asFrmNumeroCoordonnees,
                       TableauDeConstantes(NumPieton, cv->Tram[p->NumTram - 1].x, cv->Tram[p->NumTram - 1].y));
        }
        else if (p->NumTaxi != 0)
        {
            GroupBoxPieton->Caption =
                GroupBoxPieton->Caption +
                Format(asFrmNumeroCoordonnees,
                       TableauDeConstantes(NumPieton, cv->Taxi[p->NumTaxi - 1].x, cv->Taxi[p->NumTaxi - 1].y));
        }
        else if (p->NumVehlib != 0)
        {
            GroupBoxPieton->Caption =
                GroupBoxPieton->Caption +
                Format(asFrmNumeroCoordonnees,
                       TableauDeConstantes(NumPieton, cv->Vehlib[p->NumVehlib - 1].x, cv->Vehlib[p->NumVehlib - 1].y));
        }
        else
        { // à pied
            GroupBoxPieton->Caption =
                GroupBoxPieton->Caption + Format(asFrmNumeroCoordonnees, TableauDeConstantes(NumPieton, p->x, p->y));
        }
        GroupBoxPieton->Caption = GroupBoxPieton->Caption + " ";
    }
    RadioButtonAucunPieton->Enabled = !ARendreVisible;
    RadioButtonAucunPieton->Checked = !ARendreVisible;
    RadioButtonPietonNumero->Checked = ARendreVisible;
    RadioButtonPietonNumero->Enabled = ARendreVisible;
    // Libellés
    for (i = 1; i < 7; i++)
    {
        (dynamic_cast<TLabel *>(FindComponent(asLabel + IntToStr(i))))->Enabled = ARendreVisible;
    }
    // Données
    LabelDirection->Visible = ARendreVisible;
    LabelPosition->Visible = ARendreVisible;
    LabelSituation->Visible = ARendreVisible;
    LabelParcours->Visible = ARendreVisible;
    LabelBusTramTaxiVehlib->Visible = ARendreVisible;          // v5.3 ajout de Vehlib dans le nom
    LabelArretBusTramFileTaxiVehlib->Visible = ARendreVisible; // v5.3 ajout de Vehlib dans le nom
    // v5.4.1 Partie Epidémie (on active aussi si épidémie activée)
    CheckBoxEpidemie->Checked = ARendreVisible && frmSimulation->EpidemieActivee;
    for (i = 7; i < 14; i++)
    { // v5.4.1 (+9)
        (dynamic_cast<TLabel *>(FindComponent(asLabel + IntToStr(i))))->Enabled = CheckBoxEpidemie->Checked;
    }
    LabelInfecte->Visible = CheckBoxEpidemie->Checked;
    LabelInfections->Visible = CheckBoxEpidemie->Checked;
    LabelRangs->Visible = CheckBoxEpidemie->Checked;
    LabelTours->Visible = CheckBoxEpidemie->Checked;
    LabelPeriodeInfectiosite->Visible = CheckBoxEpidemie->Checked;
    LabelChargeVirale->Visible = CheckBoxEpidemie->Checked;
    LabelEtatFinal->Visible = CheckBoxEpidemie->Checked;
}
//---------------------------------------------------------------------------
void TfrmInfosPieton::RafraichitInfos()
{
    int Situation = 0;
    x = frmSimulation->DrawGridSimul->Col;
    y = frmSimulation->DrawGridSimul->Row;
    v = &cv->v[x][y];
    p = nullptr;
    NumPieton = 0;
    LabelCoordonnees->Caption = Format(asFrmCoordonnees, TableauDeConstantes(x, y));
    if (frmRecherche && frmRecherche->Visible && frmRecherche->RadioButtonPieton->Checked)
    {
        NumPieton = frmRecherche->CSpinEditPieton->Value;
    }
    if ((NumPieton < 0) || (NumPieton > cv->NbPietons))
    {
        NumPieton = 0; // portage : numéro tapé hors de 1..NbPietons dans Rechercher (lecture hors de Pieton dans
                       // l'original) : pas de piéton suivi
    }
    if (cv->Pieton.empty())
    {
        NumPieton = 0; // v6.0.2 : réseau rechargé à l'arrêt, sans piétons (accès à un pointeur nul dans l'original, au
                       // clic suivant sur la carte)
    }
    if (NumPieton == 0) // Si pas de piéton suivi, on cherche parmi ceux de la case courante
    {
        if (v->NumPieton[0] != 0)
        {
            NumPieton = v->NumPieton[0];
        }
        else if (v->NumPieton[1] != 0)
        {
            NumPieton = v->NumPieton[1];
        }
    }
    else
    {
        if ((v->NumPieton[0] != NumPieton) && // Si pas trouvé de piéton
            (v->NumPieton[1] != NumPieton) && // sur la case courante ET
            ((cv->Pieton[NumPieton - 1].NumBus == 0) ||
             (v->NumBus != cv->Pieton[NumPieton - 1].NumBus)) && // v6.0.3 : un piéton à pied (0) « était » dans le
                                                                 // véhicule d'une case qui n'en a pas (0)
            ((cv->Pieton[NumPieton - 1].NumTram == 0) || (v->NumTram != cv->Pieton[NumPieton - 1].NumTram)) &&
            ((cv->Pieton[NumPieton - 1].NumTaxi == 0) || (v->NumTaxi != cv->Pieton[NumPieton - 1].NumTaxi)) &&
            ((cv->Pieton[NumPieton - 1].NumVehlib == 0) ||
             (v->NumVehlib != cv->Pieton[NumPieton - 1].NumVehlib))) // qu'il n'y en a pas dans un bus/tram/taxi/vehlib
                                                                     // qui se trouverait sur la case. v3.6. v5.3
                                                                     // (vehlib)
        {                                                            // On cherche un autre piéton
            if (v->NumPieton[0] != 0)
            {
                NumPieton = v->NumPieton[0];
            }
            else if (v->NumPieton[1] != 0)
            {
                NumPieton = v->NumPieton[1];
            }
        }
    }
    if (NumPieton != 0)
    {
        p = &cv->Pieton[NumPieton - 1];
        LabelDirection->Caption = asDir[p->Dir];
        if (p->PsgPtTrv)
        {
            Situation = PSGPTTRV;
        }
        if (p->ArretBusTrv)
        {
            Situation = ARRETBUSTRV;
        }
        if (p->ArretTramTrv)
        {
            Situation = ARRETTRAMTRV;
        }
        if (p->FileTaxiTrv)
        {
            Situation = FILETAXITRV;
        }
        if (p->PlaceVehlibTrv)
        { // v5.3. v6.0.2 : l'original testait FileTaxiTrv
            Situation = PLACEVEHLIBTRV;
        }

        if (p->AttendBus)
        {
            Situation = ATTENDUNBUS;
        }
        if (p->NumBus != 0)
        {
            Situation = ATTENDDSLEBUS;
        }

        if (p->AttendTram)
        {
            Situation = ATTENDUNTRAM;
        }
        if (p->NumTram != 0)
        {
            Situation = ATTENDDSLETRAM;
        }

        if (p->AttendTaxi)
        {
            Situation = ATTENDUNTAXI;
        }
        if (p->NumTaxi != 0)
        {
            Situation = ATTENDDSLETAXI;
        }

        if (p->AttendVehlib)
        { // v5.3
            Situation = ATTENDUNVEHLIB;
        }
        if (p->NumVehlib != 0)
        { // v5.3
            Situation = ATTENDDSVEHLIB;
        }

        if (p->VeutDescendre)
        {
            Situation = VEUTDESCENDRE;
        }

        if (p->Ecrase)
        {
            Situation = ECRASE; // v4.3
        }

        if (p->Mort)
        {
            Situation = MORT; // v5.4.1
        }

        LabelSituation->Caption = asSituation[Situation];
        // v1.8.4 : FormatFloat au lieu de Format
        LabelParcours->Caption = FormatFloat(asFrmDemiCase, p->DemiCasesParcourues / 2.0);
        // v5.3 ajout de Vehlib dans les noms de TLabel communs (LabelBusTramTaxiVehlib et
        // LabelArretBusTramFileTaxiVehlib)
        if (p->NumBus != 0)
        {
            LabelBusTramTaxiVehlib->Caption =
                Format(asFrmNumeroCoordonnees,
                       TableauDeConstantes(p->NumBus, cv->Bus[p->NumBus - 1].x, cv->Bus[p->NumBus - 1].y));
            LabelPosition->Caption = asPosition[DANSUNBUS];
        }
        else if (p->NumTram != 0)
        {
            LabelBusTramTaxiVehlib->Caption =
                Format(asFrmNumeroCoordonnees,
                       TableauDeConstantes(p->NumTram, cv->Tram[p->NumTram - 1].x, cv->Tram[p->NumTram - 1].y));
            LabelPosition->Caption = asPosition[DANSUNTRAM];
        }
        else if (p->NumTaxi != 0) // v3.6
        {
            LabelBusTramTaxiVehlib->Caption =
                Format(asFrmNumeroCoordonnees,
                       TableauDeConstantes(p->NumTaxi, cv->Taxi[p->NumTaxi - 1].x, cv->Taxi[p->NumTaxi - 1].y));
            LabelPosition->Caption = asPosition[DANSUNTAXI];
        }
        else if (p->NumVehlib != 0) // v5.3
        {
            LabelBusTramTaxiVehlib->Caption =
                Format(asFrmNumeroCoordonnees, TableauDeConstantes(p->NumVehlib, cv->Vehlib[p->NumVehlib - 1].x,
                                                                   cv->Vehlib[p->NumVehlib - 1].y));
            LabelPosition->Caption = asPosition[DANSVEHLIB];
        }
        else
        {
            LabelBusTramTaxiVehlib->Caption = "Aucun";
            LabelPosition->Caption = asPosition[(p->DemiCase == 0) ? 0 : 1];
        }
        if (p->NumArretBus != 0)
        {
            LabelArretBusTramFileTaxiVehlib->Caption =
                Format(asFrmNumeroCoordonnees, TableauDeConstantes(p->NumArretBus, cv->ArretBus[p->NumArretBus - 1].x,
                                                                   cv->ArretBus[p->NumArretBus - 1].y));
        }
        else if (p->NumArretTram != 0)
        {
            LabelArretBusTramFileTaxiVehlib->Caption = Format(
                asFrmNumeroCoordonnees, TableauDeConstantes(p->NumArretTram, cv->ArretTram[p->NumArretTram - 1].x,
                                                            cv->ArretTram[p->NumArretTram - 1].y));
        }
        else if (p->NumFileTaxi != 0)
        {
            LabelArretBusTramFileTaxiVehlib->Caption = Format(
                asFrmNumeroCoordonnees,
                TableauDeConstantes(
                    p->NumFileTaxi, // v6.0.2 : PlaceTeteFile() commence à 1 (l'original lisait la place suivante, hors
                                    // du tableau pour la dernière)
                    cv->PlaceTaxi[cv->PlaceTaxi[cv->FileTaxi[p->NumFileTaxi - 1].NumPlaceQueue].PlaceTeteFile() - 1].x,
                    cv->PlaceTaxi[cv->PlaceTaxi[cv->FileTaxi[p->NumFileTaxi - 1].NumPlaceQueue].PlaceTeteFile() - 1]
                        .y));
        }
        else
        {
            LabelArretBusTramFileTaxiVehlib->Caption = "Aucun";
        }

        // v5.4.1 : partie Epidémie
        if (frmSimulation->EpidemieActivee)
        {
            const bool Infection = p->EstContagieux();
            const int NbInfections = p->NbInfections();
            LabelInfecte->Caption = asOuiNon[Infection ? 1 : 0];
            LabelInfections->Caption = IntToStr(NbInfections); // portage : conversion implicite int -> AnsiString de
                                                               // C++Builder
            if (NbInfections > 0)
            {
                const int Rang = p->DonnePrmRang();
                const int DrnRang = p->DonneRang();              // v5.4.2 : au lieu de DonneRang()
                const int PrmTourDeb = p->DonneTourDeb(PREMIER); // v5.4.2 : au lieu de DonneDrnRang()
                const int DrnTourDeb = p->DonneTourDeb(DERNIER);
                const int DrnTourFin = p->DonneTourFin();
                LabelRangs->Caption = Format(asFrmPrmDrn, TableauDeConstantes(Rang, DrnRang));
                LabelTours->Caption = Format(asFrmPrmDrn, TableauDeConstantes(PrmTourDeb, DrnTourDeb));
                if (Infection && (DrnTourFin - DrnTourDeb > 0))
                {
                    LabelPeriodeInfectiosite->Caption =
                        FormatFloat(asFrmPourcents, 100.0 * (cv->TourCrt - DrnTourDeb) / (DrnTourFin - DrnTourDeb));
                }
                else
                {
                    LabelPeriodeInfectiosite->Caption = asNA;
                }
                if (Infection)
                {
                    LabelEtatFinal->Caption = asEtatFinal[p->EstCondamne() ? 1 : 0];
                }
                else
                {
                    LabelEtatFinal->Caption = asNA;
                }
            }
            else
            {
                LabelRangs->Caption = asNA;
                LabelTours->Caption = asNA;
                LabelPeriodeInfectiosite->Caption = asNA;
                LabelEtatFinal->Caption = asNA;
            }
            if (cv->EpidemieChargeFatale != 0)
            {
                LabelChargeVirale->Caption =
                    FormatFloat(asFrmPourcents, (100.0 * p->DonneChargeVirale()) / cv->EpidemieChargeFatale);
            }
            else
            {
                LabelChargeVirale->Caption = asNA;
            }
        }
    }
    RendreVisibleInfos(NumPieton > 0);
}
//---------------------------------------------------------------------------
void TfrmInfosPieton::FormShow(TObject * /*Sender*/)
{
    frmSimulation->ActionDetailPieton->Checked = true;
    RafraichitInfos();
}
//---------------------------------------------------------------------------
void TfrmInfosPieton::FormHide(TObject * /*Sender*/)
{
    frmSimulation->ActionDetailPieton->Checked = false;
}
//---------------------------------------------------------------------------
void TfrmInfosPieton::ButtonOkClick(TObject * /*Sender*/)
{
    Hide();
}
//---------------------------------------------------------------------------
void TfrmInfosPieton::ButtonAideClick(TObject * /*Sender*/)
{                                  // v3.0
    AfficheAideContextuelle(this); // v3.8
}
//---------------------------------------------------------------------------

void TfrmInfosPieton::FormKeyDown(TObject * /*Sender*/, WORD &Key, TShiftState /*Shift*/)
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
    Connecte(Composant->Evenement, this, &TfrmInfosPieton::Gestionnaire);
// NOLINTEND(bugprone-macro-parentheses)
void TfrmInfosPieton::ChargeFiche()
{
    Ui::frmInfosPieton().setupUi(this);
    LieurComposants Lieur(this, "f_infptn", Composants);
    COMPOSANTS_frmInfosPieton(LIE_COMPOSANT) FinChargement(Lieur);
    EVENEMENTS_frmInfosPieton(CONNECTE_EVENEMENT) if (OnCreate)
    {
        OnCreate(this);
    }
}
//---------------------------------------------------------------------------
