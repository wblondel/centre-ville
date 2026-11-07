//---------------------------------------------------------------------------
#include "compat/chaine.h"
#include <array>
#include "vcl.h"
#include "vcl/fiche.h" // portage : remplace <vcl.h>
// #pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "base.h"
#include "simul.h"
#include "f_afdistp.h"
#include "f_simul.h"
//---------------------------------------------------------------------------
#include "ui_f_afdistp.h"         // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_afdistp_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmAffDistParkings *frmAffDistParkings;
//---------------------------------------------------------------------------
constexpr std::array<const char *, NBDIR + 1> asDir1 = {"", "   ­", "   ®", "   ¯", "   ¬"};
//---------------------------------------------------------------------------
TfrmAffDistParkings::TfrmAffDistParkings(TComponent *Owner) : TForm(Owner)
{
    ChargeFiche();
} // portage : chargement du .dfm, puis OnCreate
//---------------------------------------------------------------------------
void TfrmAffDistParkings::EffaceInfos()
{
    int i = 0;
    const int NbCol = StringGridDistances->ColCount;
    const int NbLig = StringGridDistances->RowCount;
    const int NbCases = NbCol * NbLig;
    for (i = 0; i < NbCases; i++)
    {
        StringGridDistances->Cells[i % NbCol][i / NbCol] = "";
    }
}
//---------------------------------------------------------------------------
void TfrmAffDistParkings::RafraichitInfos()
{
    int i = 0;
    int NbCases = 0;
    int NbX = 0;
    int de = 0;
    const int x = frmSimulation->DrawGridSimul->Col;
    const int y = frmSimulation->DrawGridSimul->Row;
    voie *v = &cv->v[x][y];
    EffaceInfos();
    if (v->DistParking().empty())
    {
        return;
    }
    NbCases = (NbX = 1 + NbDir[v->DirPoss]) * (1 + cv->NbParkings);
    Caption = "Parkings " + Format(asFormatCoord, TableauDeConstantes(x, y));
    for (i = 0; i < NbCases; i++)
    {
        if (i != 0)
        {
            if ((i % NbX) == 0)
            {
                StringGridDistances->Cells[i % NbX][i / NbX] = Format("%5d", TableauDeConstantes(i / NbX));
            }
            if ((i != 0) && (i < NbX))
            {
                StringGridDistances->Cells[i % NbX][i / NbX] = asDir1[Dirdp[i - 1][v->DirPoss]];
            }
            if (((i % NbX) != 0) && (i >= NbX))
            { // v3.5 // v5.4 : conversion BCB6>BCBX (AnsiString>String)
                StringGridDistances->Cells[i % NbX][i / NbX] =
                    ((de = v->DistParking()[(i / NbX) - 1][(i % NbX) - 1][CheckBoxCouloirsBus->Checked ? 1 : 0]) ==
                     DIST_INDEFINIE)
                        ? String("     ")
                        : Format("%5d", TableauDeConstantes(de));
            }
        }
    }
}
//---------------------------------------------------------------------------
void TfrmAffDistParkings::FormShow(TObject * /*Sender*/)
{
    frmSimulation->ActionDistancesParkings->Checked = true;
    StringGridDistances->RowCount = 1 + cv->NbParkings;
    StringGridDistances->FixedRows = 1; // v6.0.2 : RowCount=1 (tableau vide) l'avait ramené à 0, pour toutes les cartes
                                        // suivantes (borné à RowCount-1)
    Height = (cv->NbParkings > 10) ? 300 : (43 + (21 * StringGridDistances->RowCount));
    RafraichitInfos();
}
//---------------------------------------------------------------------------
void TfrmAffDistParkings::FormHide(TObject * /*Sender*/)
{
    frmSimulation->ActionDistancesParkings->Checked = false;
}
//---------------------------------------------------------------------------
void TfrmAffDistParkings::CheckBoxCouloirsBusClick(TObject * /*Sender*/)
{
    RafraichitInfos();
}
//---------------------------------------------------------------------------

void TfrmAffDistParkings::FormKeyDown(TObject * /*Sender*/, WORD &Key, TShiftState /*Shift*/)
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
    Connecte(Composant->Evenement, this, &TfrmAffDistParkings::Gestionnaire);
// NOLINTEND(bugprone-macro-parentheses)
void TfrmAffDistParkings::ChargeFiche()
{
    Ui::frmAffDistParkings().setupUi(this);
    LieurComposants Lieur(this, "f_afdistp", Composants);
    COMPOSANTS_frmAffDistParkings(LIE_COMPOSANT) FinChargement(Lieur);
    EVENEMENTS_frmAffDistParkings(CONNECTE_EVENEMENT) if (OnCreate)
    {
        OnCreate(this);
    }
}
//---------------------------------------------------------------------------
