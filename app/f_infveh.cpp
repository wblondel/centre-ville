//---------------------------------------------------------------------------
#include "compat/chaine.h"
#include <array>
#include "simul.h"
#include "vcl.h"
#include "vcl/fiche.h" // portage : remplace <vcl.h>
// #pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "f_infveh.h"
#include "f_simul.h"
#include "base.h"
//---------------------------------------------------------------------------
#include "ui_f_infveh.h"         // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_infveh_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmInfosVehicule *frmInfosVehicule;
//---------------------------------------------------------------------------
constexpr const char *asDebutTitreCadreVehicule = "                           ";
constexpr const char *asFrmNumeroCoordonnees = "%d (%2.2d, %2.2d)";
constexpr const char *asFrmCoordonnees = "%2.2d, %2.2d";
constexpr const char *asFrmPourcentage = "#.000%"; // v3.0. v5.4.1 : FormatFloat n'utilise pas le format "%.3f%%" mais
                                                   // // "#.000%"
constexpr const char *asFrmVitesse = "%d/%d";
constexpr std::array<const char *, NBNIVEAUXPRIORITE> asNiveauPriorite = {"Normal", "Urgences", "Police"};
constexpr std::array<const char *, 2> asDepartArrivee = {"de départ", "d'arrivée"};           // v5.0
constexpr std::array<const char *, 2> asFrmFileParking = {"Parking %s :", "File Parking %s"}; // v5.0
//---------------------------------------------------------------------------
const TColor CouleurNiveauPriorite[NBNIVEAUXPRIORITE] = {clBlack, clRed, clBlue};
//---------------------------------------------------------------------------
TfrmInfosVehicule::TfrmInfosVehicule(TComponent *Owner) : TForm(Owner)
{
    ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
}
//---------------------------------------------------------------------------
void TfrmInfosVehicule::RendreVisibleInfos(bool ARendreVisible)
{
    int i = 0;
    GroupBoxVehicule->Caption = asDebutTitreCadreVehicule;
    if (ARendreVisible)
    {
        RadioButtonVehiculeNumero->Checked = true;
        GroupBoxVehicule->Caption =
            GroupBoxVehicule->Caption +
            Format(asFrmNumeroCoordonnees, TableauDeConstantes(v->NumVehicule, Veh->x, Veh->y)) + " ";
    }
    else
    {
        RadioButtonAucunVehicule->Checked = true;
    }
    RadioButtonVehiculeNumero->Enabled = ARendreVisible;
    RadioButtonAucunVehicule->Enabled = !ARendreVisible;
    for (i = 3; i < 14; i++)
    { // v5.0 : i=3 au lieu de i=1
        (dynamic_cast<TLabel *>(FindComponent("Label" + IntToStr(i))))->Enabled = ARendreVisible;
    }
    LabelDepart->Visible = ARendreVisible;  // v5.0
    LabelArrivee->Visible = ARendreVisible; // v5.0
    LabelDirection->Visible = ARendreVisible;
    LabelNivPriorite->Visible = ARendreVisible;
    LabelParkingDepart->Visible = ARendreVisible;
    LabelParkingArrivee->Visible = ARendreVisible;
    LabelTourSortie->Visible = ARendreVisible;
    LabelTours->Visible = ARendreVisible;
    LabelParcours->Visible = ARendreVisible;
    LabelDistance->Visible = ARendreVisible;
    LabelDistanceMin->Visible = ARendreVisible;
    LabelParcoursEffectue->Visible = ARendreVisible;
    LabelDetour->Visible = ARendreVisible;
    LabelAttente->Visible = ARendreVisible;
    LabelVitesse->Visible = ARendreVisible;
}
//---------------------------------------------------------------------------
void TfrmInfosVehicule::RafraichitInfos()
{
    int i = 0;
    int NbTours = 0;
    int NumParkingDepart = 0;
    int NumParkingArrivee = 0;
    int NumFileParkDepart = 0;
    int NumFileParkArrivee = 0;
    int // v5.0
        NbDirPoss = 0;
    bool Prioritaire = false;
    int DejaParcouru = 0;
    int ResteAParcourir = 0;
    int DistMin = 0;
    voie *CaseDepart = nullptr; // v5.0 : ATTENTION, le type de parking peut être souterrain ou extérieur !
    bool Afficher = false;
    x = frmSimulation->DrawGridSimul->Col;
    y = frmSimulation->DrawGridSimul->Row;
    v = &cv->v[x][y];
    Veh = nullptr;
    LabelCoordonnees->Caption = Format(asFrmCoordonnees, TableauDeConstantes(x, y));
    if ((Afficher = (v->NumVehicule > 0)))
    {
        Veh = cv->Vehicule + v->NumVehicule - 1;
        NumParkingDepart = Veh->NumParkingDepart;     // v5.0
        NumParkingArrivee = Veh->NumParkingArrivee;   // v5.0
        NumFileParkDepart = Veh->NumFileParkDepart;   // v5.0
        NumFileParkArrivee = Veh->NumFileParkArrivee; // v5.0
        Prioritaire = (Veh->NivPriorite > normal);
        LabelDirection->Caption = asDir[Veh->Dir];

        LabelNivPriorite->Caption = asNiveauPriorite[Veh->NivPriorite];
        LabelNivPriorite->Font->Color = CouleurNiveauPriorite[Veh->NivPriorite];

        if (NumFileParkDepart != 0) // v5.0
        {
            LabelDepart->Caption = Format(asFrmFileParking[FILEPARK], TableauDeConstantes(asDepartArrivee[DEPART]));
            LabelParkingDepart->Caption =
                Format(asFrmNumeroCoordonnees,
                       TableauDeConstantes(NumFileParkDepart,
                                           cv->PlacePark[cv->FilePark[NumFileParkDepart - 1].NumPlaceQueue].x,
                                           cv->PlacePark[cv->FilePark[NumFileParkDepart - 1].NumPlaceQueue].y));
        }
        else
        {
            LabelDepart->Caption = Format(asFrmFileParking[PARKING], TableauDeConstantes(asDepartArrivee[DEPART]));
            LabelParkingDepart->Caption = Format(
                asFrmNumeroCoordonnees, TableauDeConstantes(NumParkingDepart, cv->Parking[NumParkingDepart - 1].x,
                                                            cv->Parking[NumParkingDepart - 1].y));
        }

        if (NumFileParkArrivee != 0) // v5.0
        {
            LabelArrivee->Caption = Format(asFrmFileParking[FILEPARK], TableauDeConstantes(asDepartArrivee[ARRIVEE]));
            LabelParkingArrivee->Caption =
                Format(asFrmNumeroCoordonnees,
                       TableauDeConstantes(NumFileParkArrivee,
                                           cv->PlacePark[cv->FilePark[NumFileParkArrivee - 1].NumPlaceQueue].x,
                                           cv->PlacePark[cv->FilePark[NumFileParkArrivee - 1].NumPlaceQueue].y));
        }
        else
        {
            LabelArrivee->Caption = Format(asFrmFileParking[PARKING], TableauDeConstantes(asDepartArrivee[ARRIVEE]));
            LabelParkingArrivee->Caption = Format(
                asFrmNumeroCoordonnees, TableauDeConstantes(NumParkingArrivee, cv->Parking[NumParkingArrivee - 1].x,
                                                            cv->Parking[NumParkingArrivee - 1].y));
        }

        LabelTourSortie->Caption = IntToStr(Veh->TourDepart);
        LabelTours->Caption = IntToStr(NbTours = cv->TourCrt - Veh->TourDepart);
        LabelParcours->Caption = IntToStr(DejaParcouru = Veh->NbCasesParcourues);
        if (NumParkingArrivee != 0)
        { // v5.0
            LabelDistance->Caption = IntToStr(
                ResteAParcourir =
                    v->DistParking[NumParkingArrivee - 1][NumDirdp[Veh->Dir][v->DirPoss]][Prioritaire]); // v3.5
        }
        if (NumParkingDepart != 0)
        { // v5.0
            CaseDepart = &cv->v[cv->Parking[NumParkingDepart - 1].x][cv->Parking[NumParkingDepart - 1].y];
        }
        if (NumFileParkArrivee != 0)
        { // v5.0
            LabelDistance->Caption = IntToStr(
                ResteAParcourir =
                    v->DistFilePark[NumFileParkArrivee - 1][NumDirdp[Veh->Dir][v->DirPoss]][Prioritaire]); // v3.5
        }
        if (NumFileParkDepart != 0)
        { // v5.0
            CaseDepart = &cv->v[cv->PlacePark[cv->FilePark[NumFileParkDepart - 1].NumPlaceQueue].x]
                               [cv->PlacePark[cv->FilePark[NumFileParkDepart - 1].NumPlaceQueue].y];
        }
        NbDirPoss = NbDir[CaseDepart->DirPoss];
        if (NumFileParkArrivee != 0)
        { // v5.0
            for (i = 0, DistMin = DIST_INDEFINIE; i < NbDirPoss; i++)
            {
                DistMin = min(DistMin, CaseDepart->DistFilePark[NumFileParkArrivee - 1][i][Prioritaire]); // v5.0
            }
        }
        if (NumParkingArrivee != 0)
        {
            for (i = 0, DistMin = DIST_INDEFINIE; i < NbDirPoss; i++)
            {
                DistMin = min(DistMin, CaseDepart->DistParking[NumParkingArrivee - 1][i][Prioritaire]); // v3.5
            }
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
        LabelVitesse->Caption =
            Format(asFrmVitesse, TableauDeConstantes(1 + ((v->NumVehicule - 1) % cv->NbVitesses), cv->NbVitesses));
    }
    RendreVisibleInfos(Afficher);
}
//---------------------------------------------------------------------------
void TfrmInfosVehicule::FormShow(TObject * /*Sender*/)
{
    frmSimulation->ActionDetailVehicule->Checked = true;
    RafraichitInfos();
}
//---------------------------------------------------------------------------
void TfrmInfosVehicule::FormHide(TObject * /*Sender*/)
{
    frmSimulation->ActionDetailVehicule->Checked = false;
}
//---------------------------------------------------------------------------
void TfrmInfosVehicule::ButtonOkClick(TObject * /*Sender*/)
{
    Hide();
}
//---------------------------------------------------------------------------
void TfrmInfosVehicule::ButtonAideClick(TObject * /*Sender*/)
{                                  // v3.0
    AfficheAideContextuelle(this); // v3.8
}
//---------------------------------------------------------------------------

void TfrmInfosVehicule::FormKeyDown(TObject * /*Sender*/, WORD &Key, TShiftState /*Shift*/)
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
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire)                                                         \
    Connecte(Composant->Evenement, this, &TfrmInfosVehicule::Gestionnaire);
void TfrmInfosVehicule::ChargeFiche()
{
    Ui::frmInfosVehicule().setupUi(this);
    LieurComposants Lieur(this, "f_infveh", Composants);
    COMPOSANTS_frmInfosVehicule(LIE_COMPOSANT) FinChargement(Lieur);
    EVENEMENTS_frmInfosVehicule(CONNECTE_EVENEMENT) if (OnCreate)
    {
        OnCreate(this);
    }
}
//---------------------------------------------------------------------------
