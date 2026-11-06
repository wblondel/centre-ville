//---------------------------------------------------------------------------
#include "base.h"
#include "compat/chaine.h"
#include "controles.h"
#include "vcl.h"
#include "vcl/fiche.h" // portage : remplace <vcl.h>
// #pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "simul.h"
#include "f_statsat.h"

#include <algorithm>
#include "f_simul.h"
//---------------------------------------------------------------------------
#include "ui_f_statsat.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
// #pragma link "cspin" // portage
#include "f_statsat_composants.h" // portage : #pragma resource "*.dfm"
#include "vue.h"
//---------------------------------------------------------------------------
constexpr int ATTENTE = 0;
constexpr int TRAFIC = 1;
//---------------------------------------------------------------------------
TfrmStatsAttenteTrafic *frmStatsAttenteTrafic;
//---------------------------------------------------------------------------
TfrmStatsAttenteTrafic::TfrmStatsAttenteTrafic(TComponent *Owner) : TForm(Owner)
{
    m[0] = 0;
    m[1] = 0;
    ChargeFiche(); // portage : membres à zéro (TObject::NewInstance), chargement du .dfm, puis OnCreate
    // v5.4 on charge les glyphes manuellement pour prendre en compte le HDPI
    for (int i = 0; i < 9; i++)
    {
        if (i != 4)
        { // v5.4 : plus de zoom !
            frmSimulation->AffecteGlyphe(GLYPHE_DIR + i,
                                         (dynamic_cast<TBitBtn *>(FindComponent(
                                              UnicodeString("BitBtn" + IntToStr(1 + (i % 3)) + IntToStr(1 + (i / 3))))))
                                             ->Glyph);
        }
    }
    // v6.0.2 : dans ces deux champs, les touches de déplacement servent au champ. Les
    // Paramètres des bus et des trams gardent les raccourcis : leurs contrôles sont
    // presque tous des champs, et ils y déplacent le curseur pour montrer un arrêt
    CSpinEditParkingDepart->GardeTouchesDeplacement = true;
    CSpinEditParkingArrivee->GardeTouchesDeplacement = true;
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::PaintBoxPaint(TObject * /*Sender*/)
{
    for (int i = 0; i < 128; i++)
    {
        PaintBox->Canvas->Pen->Color =
            static_cast<TColor>(RGB((i < 64) ? 4 * i : 252, (i >= 64) ? 252 - (4 * (i - 64)) : 252, 0));
        PaintBox->Canvas->MoveTo(i, 0);
        PaintBox->Canvas->LineTo(i, PaintBox->Height - 1);
    }
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::FormShow(TObject * /*Sender*/)
{
    if (cv->TourCrt != 0)
    {
        Caption = "Statistiques de fluidité et de trafic au tour n°" + IntToStr(cv->TourCrt);
    }
    CSpinEditParkingDepart->MaxValue = cv->NbParkings;
    CSpinEditParkingArrivee->MaxValue = cv->NbParkings;
    RecalculeEtVisualise();
}
//---------------------------------------------------------------------------
TColor TfrmStatsAttenteTrafic::CouleurCase(int x, int y)
{
    int s = 0;
    const int ed = CheckBoxParkingDepart->Checked ? CSpinEditParkingDepart->Value : 0;
    const int ea = CheckBoxParkingArrivee->Checked ? CSpinEditParkingArrivee->Value : 0;
    const int ts = RadioGroupType->ItemIndex;
    if ((ed < 0) || (ed > cv->NbParkings) || (ea < 0) || (ea > cv->NbParkings))
    {
        return static_cast<TColor>(0); // portage : n° d'étape tapé hors bornes (lecture hors de Attente/Trafic dans
                                       // l'original)
    }
    if ((m[ts] != 0) && !cv->v[x][y].Trafic.empty() && !cv->v[x][y].Attente.empty()) // v3.0
    {
        if (ts != 0)
        {
            s = (127 * cv->v[x][y].Trafic[ed][ea]) / m[ts];
        }
        else
        {
            s = (127 * cv->v[x][y].Attente[ed][ea]) / m[ts];
        }
        return static_cast<TColor>(RGB((s < 64) ? 4 * s : 252, (s >= 64) ? 252 - (4 * (s - 64)) : 252, 0));
    }

    return static_cast<TColor>(0);
}
//---------------------------------------------------------------------------
AnsiString TfrmStatsAttenteTrafic::asMsgDetailCase(int x, int y)
{ // v3.0
    AnsiString asMsg;
    const int ed = CheckBoxParkingDepart->Checked ? CSpinEditParkingDepart->Value : 0;
    const int ea = CheckBoxParkingArrivee->Checked ? CSpinEditParkingArrivee->Value : 0;
    const int ts = RadioGroupType->ItemIndex;
    if ((ed < 0) || (ed > cv->NbParkings) || (ea < 0) || (ea > cv->NbParkings))
    {
        return asMsg; // portage : n° d'étape tapé hors bornes (lecture hors de Attente/Trafic dans l'original)
    }
    if ((m[ts] != 0) && !cv->v[x][y].Trafic.empty() && !cv->v[x][y].Attente.empty())
    {
        if (ts != 0)
        {
            asMsg = Format("Trafic en (%d, %d) au tour %d : %d véhicules (Max. %d) soit %.3f%%",
                           TableauDeConstantes(x, y, cv->TourCrt, cv->v[x][y].Trafic[ed][ea], m[ts],
                                               static_cast<long double>((100.0 * cv->v[x][y].Trafic[ed][ea]) / m[ts])));
        }
        else
        {
            asMsg =
                Format("Attente en (%d, %d) au tour %d : %d tours (Max. %d) soit %.3f%%",
                       TableauDeConstantes(x, y, cv->TourCrt, cv->v[x][y].Attente[ed][ea], m[ts],
                                           static_cast<long double>((100.0 * cv->v[x][y].Attente[ed][ea]) / m[ts])));
        }
    }
    return asMsg;
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::RecalculeEtVisualise()
{
    int x = 0;
    int y = 0;
    int n = 0;
    int t[2] = {0, 0};
    const int ed = CheckBoxParkingDepart->Checked ? CSpinEditParkingDepart->Value : 0;
    const int ea = CheckBoxParkingArrivee->Checked ? CSpinEditParkingArrivee->Value : 0;
    if ((ed < 0) || (ed > cv->NbParkings) || (ea < 0) || (ea > cv->NbParkings))
    {
        AfficheMessage("Le n° d'étape est incorrect !", "Valeur incorrecte", tmAvertissement, 0, 0);
        return;
    }
    m[0] = 0;
    m[1] = 0;
    for (x = 0; x < cv->NbX; x++)
    {
        for (y = 0; y < cv->NbY; y++)
        {
            if ((cv->v[x][y].DirPoss > aucune) && (cv->v[x][y].NumParking == 0))
            {
                n++;
                if (!cv->v[x][y].Attente.empty() && !cv->v[x][y].Trafic.empty())
                {
                    m[ATTENTE] = std::max(cv->v[x][y].Attente[ed][ea], m[ATTENTE]);
                    t[ATTENTE] += cv->v[x][y].Attente[ed][ea];
                    m[TRAFIC] = std::max(cv->v[x][y].Trafic[ed][ea], m[TRAFIC]);
                    t[TRAFIC] += cv->v[x][y].Trafic[ed][ea];
                }
            }
        }
    }
    LabelMax->Caption = IntToStr(m[RadioGroupType->ItemIndex]);
    LabelAttenteCumulee->Caption = IntToStr(t[ATTENTE]);
    if (n != 0)
    { // v1.8.4 : FormatFloat à la place de Format
        LabelAttenteMoyenne->Caption = FormatFloat("0.0", 1.0 * t[ATTENTE] / n); // v3.0 (ajout). v5.4.1 : FormatFloat
                                                                                 // n'utilise pas le format "%.1f" mais
                                                                                 // "#.0"
    }
    else
    {
        LabelAttenteMoyenne->Caption = "N/D";
    }
    LabelAttenteMaximale->Caption = IntToStr(m[ATTENTE]);
    LabelTraficCumule->Caption = IntToStr(t[TRAFIC]);
    if (n != 0)
    { // v1.8.4 : FormatFloat à la place de Format
        LabelTraficMoyen->Caption = FormatFloat("0.0", 1.0 * t[TRAFIC] / n); // v5.4.1 : FormatFloat n'utilise pas le
                                                                             // format "%.1f" mais "#.0"
    }
    else
    {
        LabelTraficMoyen->Caption = "N/D";
    }
    LabelTraficMaximal->Caption = IntToStr(m[TRAFIC]);
    frmSimulation->DrawGridSimul->Repaint();
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::ParametresParkingsClick(TObject *Sender)
{
    switch (TagDe(static_cast<TComponent *>(Sender))) // Comportements propres à certains objets // portage :
                                                      // TComponent::Tag -> TagDe
    {
    case 1:
        CSpinEditParkingDepart->Enabled = CheckBoxParkingDepart->Checked;
        if (CSpinEditParkingDepart->Enabled)
        {
            CSpinEditParkingDepart->SetFocus();
        }
        break;
    case 2:
        CSpinEditParkingArrivee->Enabled = CheckBoxParkingArrivee->Checked;
        if (CSpinEditParkingArrivee->Enabled)
        {
            CSpinEditParkingArrivee->SetFocus();
        }
        break;
    default:
        break;
    }
    RecalculeEtVisualise();
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::RadioGroupTypeClick(TObject * /*Sender*/)
{
    LabelMax->Caption = IntToStr(m[RadioGroupType->ItemIndex]);
    frmSimulation->DrawGridSimul->Repaint();
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::BitBtnDirClick(TObject *Sender)
{
    const int n = StrToInt(
        NomDe(static_cast<TComponent *>(Sender)).SubString(7, NomDe(static_cast<TComponent *>(Sender)).Length() - 6));
    int // portage : TComponent::Name -> NomDe
        const dx = (n / 10) - 2;
    const int dy = (n % 10) - 2;
    if (CheckBoxCoins->Checked)
    {
        switch (dx)
        {
        case -1:
            frmSimulation->DrawGridSimul->Col = 0;
            break;
        case +1:
            frmSimulation->DrawGridSimul->Col = frmSimulation->DrawGridSimul->ColCount - 1;
            break;
        default:
            break;
        }
        switch (dy)
        {
        case -1:
            frmSimulation->DrawGridSimul->Row = 0;
            break;
        case +1:
            frmSimulation->DrawGridSimul->Row = frmSimulation->DrawGridSimul->RowCount - 1;
            break;
        default:
            break;
        }
    }
    else
    {
        if ((frmSimulation->DrawGridSimul->Col + dx >= 0) &&
            (frmSimulation->DrawGridSimul->Col + dx < frmSimulation->DrawGridSimul->ColCount))
        {
            frmSimulation->DrawGridSimul->Col += dx;
        }
        if ((frmSimulation->DrawGridSimul->Row + dy >= 0) &&
            (frmSimulation->DrawGridSimul->Row + dy < frmSimulation->DrawGridSimul->RowCount))
        {
            frmSimulation->DrawGridSimul->Row += dy;
        }
    }
}
//---------------------------------------------------------------------------
void TfrmStatsAttenteTrafic::RubriqueClick(TObject *Sender)
{
    TBitBtn *BitBtn = dynamic_cast<TBitBtn *>(
        FindComponent("BitBtn" + IntToStr(TagDe(static_cast<TComponent *>(Sender))))); // portage
                                                                                       // :
                                                                                       // TComponent::Tag
                                                                                       // ->
                                                                                       // TagDe
    if (BitBtn)
    {
        BitBtn->Click(); // portage : « Zoom » (Tag 22) : pas de BitBtn22 depuis la v5.4, violation d'accès dans
                         // l'original
    }
}
//---------------------------------------------------------------------------

void TfrmStatsAttenteTrafic::FormKeyDown(TObject * /*Sender*/, WORD &Key, TShiftState /*Shift*/)
{ // v5.3 (nécessite TForm:KeyPreview à true)
    if (Key == 112)
    {
        AfficheAideContextuelle(this);
    }
    if (Key == 27)
    { // v6.0.2 : la fiche n'a pas de bouton Cancel, et Échap ne la fermait pas
        Close();
    }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
// NOLINTBEGIN(bugprone-macro-parentheses) : noms du composant, de l'événement et du gestionnaire
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire)                                                         \
    Connecte(Composant->Evenement, this, &TfrmStatsAttenteTrafic::Gestionnaire);
// NOLINTEND(bugprone-macro-parentheses)
void TfrmStatsAttenteTrafic::ChargeFiche()
{
    Ui::frmStatsAttenteTrafic().setupUi(this);
    LieurComposants Lieur(this, "f_statsat", Composants);
    COMPOSANTS_frmStatsAttenteTrafic(LIE_COMPOSANT) FinChargement(Lieur);
    EVENEMENTS_frmStatsAttenteTrafic(CONNECTE_EVENEMENT) if (OnCreate)
    {
        OnCreate(this);
    }
}
//---------------------------------------------------------------------------
