//---------------------------------------------------------------------------
#include "base.h"
#include "compat/chaine.h"
#include "simul.h"
#include "vcl.h"
#include "vcl/fiche.h" // portage : remplace <vcl.h>
// #pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "f_simul.h"
#include "f_deff.h"
//---------------------------------------------------------------------------
#include "ui_f_deff.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
// #pragma link "cspin" // portage : TCSpinEdit dans vcl/controles.h
#include "f_deff_composants.h" // portage : #pragma resource "*.dfm"
#include <vector>
//---------------------------------------------------------------------------
TfrmDefFeux *frmDefFeux = nullptr;
//---------------------------------------------------------------------------
TfrmDefFeux::TfrmDefFeux(TComponent *Owner) : TForm(Owner)
{
    ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
    NbFeuxDmd = 0;
    NbFeuxAtt = 0;
    // v5.4 on charge les glyphes manuellement pour prendre en compte le HDPI
    frmSimulation->AffecteGlyphe(GLYPHE_SELF, SpeedButtonSelFeuxAtt->Glyph);
    frmSimulation->AffecteGlyphe(GLYPHE_SELF, SpeedButtonSelFeuxDmd->Glyph);
}
//---------------------------------------------------------------------------
bool TfrmDefFeux::RafraichitInfos()
{
    int i = 0;
    int NbFeuxP = 0;
    v = &cv->v[x][y];
    Numero = v->NumFeu[Direction - 1];
    if (Numero == 0)
    {
        Numero = cv->AjouteFeu(x, y, Direction); // v5.2 : plus de message de limite.
    }
    v->NumFeu[Direction - 1] = Numero;
    f = &cv->Feu[Numero - 1];
    LabelCoordonnees->Caption = Format("%2.2d, %2.2d", TableauDeConstantes(x, y));
    LabelDirection->Caption = asDir[Direction];
    if (Numero != 0)
    {
        LabelNumero->Caption = IntToStr(Numero);
    }
    RadioGroupEtat->ItemIndex = f->Etat;
    RadioGroupTypeDmdVert->ItemIndex = f->TypeDmdVert;
    RadioGroupTypePsgRouge->ItemIndex = f->TypePsgRouge;
    CSpinEditDureeVert->Value = f->DureeVert;
    CSpinEditDureeRouge->Value = f->DureeRouge;

    // Copie des listes de feux
    NbFeuxDmd = f->NbFeuxDmd;
    NbFeuxAtt = f->NbFeuxAtt;
    NumFeuxDmd = std::vector<int>(NbFeuxDmd);
    NumFeuxAtt = std::vector<int>(NbFeuxAtt);
    for (i = 0; i < NbFeuxDmd; i++)
    {
        NumFeuxDmd[i] = f->NumFeuxDmd[i];
    }
    for (i = 0; i < NbFeuxAtt; i++)
    {
        NumFeuxAtt[i] = f->NumFeuxAtt[i];
    }

    // Affichage du nombre de feux de chaque liste
    LabelNbFeuxDmd->Caption = IntToStr(NbFeuxDmd);
    LabelNbFeuxAtt->Caption = IntToStr(NbFeuxAtt);

    // Affichage des numéros de feux piétons liés (v5.2)
    LabelFeuxPietons->Caption = "";

    for (i = 0, NbFeuxP = 0; i < cv->NbFeuxPieton; i++)
    {
        if (cv->FeuP[i].Defini() && (cv->FeuP[i].NbFeux != 0))
        {
            for (int j = 0; j < cv->FeuP[i].NbFeux; j++)
            {
                if (cv->FeuP[i].NumFeux[j] == Numero)
                {
                    LabelFeuxPietons->Caption =
                        LabelFeuxPietons->Caption +
                        Format((LabelFeuxPietons->Caption == "") ? "%d" : ", %d", TableauDeConstantes(i + 1));
                    NbFeuxP++;
                }
            }
        }
    }
    if (LabelFeuxPietons->Caption == "")
    {
        LabelFeuxPietons->Caption = "Aucun";
    }
    else
    {
        LabelFeuxPietons->Caption = IntToStr(NbFeuxP) + " (" + LabelFeuxPietons->Caption + ")";
    }

    ButtonAppliquer->Enabled = false; // v5.2 : En cas de changement de coordonnées (boîte non modale) => Annulation
                                      // implicite donc Appliquer ne peut être actif.
    return true;
}
//---------------------------------------------------------------------------
bool TfrmDefFeux::AppliqueChoix()
{
    int i = 0;
    // v6.0.10 : la fiche n'est pas modale. Depuis RafraichitInfos, Carrefours ou la fiche
    // Priorités ont pu agrandir la table des feux, donc la déplacer ; Carrefours a pu effacer ce
    // feu et reprendre son numéro ailleurs : rien n'est alors écrit (et la fiche se ferme)
    if (cv->v[x][y].NumFeu[Direction - 1] != Numero)
    {
        return false;
    }
    f = &cv->Feu[Numero - 1];
    frmSimulation->SimulationModifiee = true;
    f->Etat = static_cast<etat_feu>(RadioGroupEtat->ItemIndex);
    f->TypeDmdVert = static_cast<dmd_vert>(RadioGroupTypeDmdVert->ItemIndex);
    f->TypePsgRouge = static_cast<psg_rouge>(RadioGroupTypePsgRouge->ItemIndex);
    f->DureeVert = CSpinEditDureeVert->Value;
    f->DureeRouge = CSpinEditDureeRouge->Value;

    // Copie des listes de feux
    f->NbFeuxDmd = 0;
    f->NbFeuxAtt = 0;
    f->NumFeuxDmd = std::vector<int>(NbFeuxDmd);
    f->NumFeuxAtt = std::vector<int>(NbFeuxAtt);
    f->NbFeuxDmd = NbFeuxDmd;
    f->NbFeuxAtt = NbFeuxAtt;
    for (i = 0; i < NbFeuxDmd; i++)
    {
        f->NumFeuxDmd[i] = NumFeuxDmd[i];
    }
    for (i = 0; i < NbFeuxAtt; i++)
    {
        f->NumFeuxAtt[i] = NumFeuxAtt[i];
    }
    ButtonAppliquer->Enabled = false;
    frmSimulation->RedessineCase(x, y, true);
    return true;
}
//---------------------------------------------------------------------------
void TfrmDefFeux::RafraichitInfosFeuxDmd()
{
    ButtonAppliquer->Enabled = true;
    LabelNbFeuxDmd->Caption = IntToStr(NbFeuxDmd);
}
//---------------------------------------------------------------------------
void TfrmDefFeux::RafraichitInfosFeuxAtt()
{
    ButtonAppliquer->Enabled = true;
    LabelNbFeuxAtt->Caption = IntToStr(NbFeuxAtt);
}
//---------------------------------------------------------------------------
void TfrmDefFeux::FormShow(TObject * /*Sender*/)
{
    if (!RafraichitInfos())
    {
        Hide();
    }
    else
    {
        frmSimulation->RedessineCase(x, y, true);
    }
}
//---------------------------------------------------------------------------
void TfrmDefFeux::FormHide(TObject * /*Sender*/)
{
    int i = 0;
    frmSimulation->ActionPriorites->Enabled = true;
    SpeedButtonSelFeuxDmd->Down = false;
    SpeedButtonSelFeuxAtt->Down = false;
    if (frmSimulation->SelectionFeuxDmd)
    {
        for (i = 0; i < NbFeuxDmd; i++)
        {
            frmSimulation->RedessineCase(cv->Feu[NumFeuxDmd[i] - 1].x, cv->Feu[NumFeuxDmd[i] - 1].y, true);
        }
    }
    if (frmSimulation->SelectionFeuxAtt)
    {
        for (i = 0; i < NbFeuxAtt; i++)
        {
            frmSimulation->RedessineCase(cv->Feu[NumFeuxAtt[i] - 1].x, cv->Feu[NumFeuxAtt[i] - 1].y, true);
        }
    }
    frmSimulation->SelectionFeuxDmd = false;
    frmSimulation->SelectionFeuxAtt = false;
    // v5.2 : En cas de dfinition de feux, on revient à la case initiale.
    frmSimulation->DrawGridSimul->Col = x;
    frmSimulation->DrawGridSimul->Row = y;
    frmSimulation->DrawGridSimulClick(frmSimulation);
}
//---------------------------------------------------------------------------
void TfrmDefFeux::ButtonOkClick(TObject * /*Sender*/)
{
    AppliqueChoix();
    Hide();
}
//---------------------------------------------------------------------------
void TfrmDefFeux::ButtonAnnulerClick(TObject * /*Sender*/)
{
    ButtonAppliquer->Enabled = false;
    Hide();
}
//---------------------------------------------------------------------------
void TfrmDefFeux::ButtonAppliquerClick(TObject * /*Sender*/)
{
    if (!AppliqueChoix())
    {
        Hide();
    }
}
//---------------------------------------------------------------------------
void TfrmDefFeux::RadioGroupEtatClick(TObject * /*Sender*/)
{
    f = &cv->Feu[Numero - 1]; // v6.0.10 : voir AppliqueChoix
    if (RadioGroupEtat->ItemIndex != f->Etat)
    {
        ButtonAppliquer->Enabled = true;
    }
}
//---------------------------------------------------------------------------
void TfrmDefFeux::RadioGroupTypeDmdVertClick(TObject * /*Sender*/)
{
    f = &cv->Feu[Numero - 1]; // v6.0.10 : voir AppliqueChoix
    if (RadioGroupTypeDmdVert->ItemIndex != f->TypeDmdVert)
    {
        ButtonAppliquer->Enabled = true;
    }
}
//---------------------------------------------------------------------------
void TfrmDefFeux::CSpinEditDureeVertChange(TObject * /*Sender*/)
{
    ButtonAppliquer->Enabled = true;
}
//---------------------------------------------------------------------------
void TfrmDefFeux::SpeedButtonSelFeuxDmdClick(TObject * /*Sender*/)
{
    int i = 0;
    if (SpeedButtonSelFeuxDmd->Down && (!SpeedButtonSelFeuxAtt->Down))
    {
        SpeedButtonSelFeuxAtt->Click();
    }
    frmSimulation->SelectionFeuxDmd = SpeedButtonSelFeuxDmd->Down;
    for (i = 0; i < NbFeuxDmd; i++)
    {
        frmSimulation->RedessineCase(cv->Feu[NumFeuxDmd[i] - 1].x, cv->Feu[NumFeuxDmd[i] - 1].y, true);
    }
    frmSimulation->RedessineCase(frmSimulation->DrawGridSimul->Col, frmSimulation->DrawGridSimul->Row, true);
    frmSimulation->ActionPriorites->Enabled = !(frmSimulation->SelectionFeuxDmd || frmSimulation->SelectionFeuxAtt);
}
//---------------------------------------------------------------------------
void TfrmDefFeux::SpeedButtonSelFeuxAttClick(TObject * /*Sender*/)
{
    int i = 0;
    if (SpeedButtonSelFeuxAtt->Down && (!SpeedButtonSelFeuxDmd->Down))
    {
        SpeedButtonSelFeuxDmd->Click();
    }
    frmSimulation->SelectionFeuxAtt = SpeedButtonSelFeuxAtt->Down;
    for (i = 0; i < NbFeuxAtt; i++)
    {
        frmSimulation->RedessineCase(cv->Feu[NumFeuxAtt[i] - 1].x, cv->Feu[NumFeuxAtt[i] - 1].y, true);
    }
    frmSimulation->RedessineCase(frmSimulation->DrawGridSimul->Col, frmSimulation->DrawGridSimul->Row, true);
    frmSimulation->ActionPriorites->Enabled = !(frmSimulation->SelectionFeuxDmd || frmSimulation->SelectionFeuxAtt);
}
//---------------------------------------------------------------------------
void TfrmDefFeux::RadioGroupTypePsgRougeClick(TObject * /*Sender*/)
{
    f = &cv->Feu[Numero - 1]; // v6.0.10 : voir AppliqueChoix
    if (RadioGroupTypePsgRouge->ItemIndex != f->TypePsgRouge)
    {
        ButtonAppliquer->Enabled = true;
    }
}
//---------------------------------------------------------------------------
void TfrmDefFeux::ButtonAideClick(TObject * /*Sender*/)
{                                  // v3.0
    AfficheAideContextuelle(this); // v3.8
}
//---------------------------------------------------------------------------
void TfrmDefFeux::FormKeyDown(TObject * /*Sender*/, WORD &Key, TShiftState /*Shift*/)
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
    Connecte(Composant->Evenement, this, &TfrmDefFeux::Gestionnaire);
// NOLINTEND(bugprone-macro-parentheses)
void TfrmDefFeux::ChargeFiche()
{
    Ui::frmDefFeux().setupUi(this);
    LieurComposants Lieur(this, "f_deff", Composants);
    COMPOSANTS_frmDefFeux(LIE_COMPOSANT) FinChargement(Lieur);
    EVENEMENTS_frmDefFeux(CONNECTE_EVENEMENT) if (OnCreate)
    {
        OnCreate(this);
    }
}
//---------------------------------------------------------------------------
