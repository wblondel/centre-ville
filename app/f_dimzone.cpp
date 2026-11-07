//---------------------------------------------------------------------------
#include "base.h"
#include "compat/chaine.h"
#include "vcl.h"
#include "vcl/fiche.h" // portage : remplace <vcl.h>
// #pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "simul.h"
#include "f_simul.h"
#include "f_dimzone.h"
//---------------------------------------------------------------------------
#include "ui_f_dimzone.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
// #pragma link "cspin" // portage : TCSpinEdit dans vcl/controles.h
#include "f_dimzone_composants.h" // portage : #pragma resource "*.dfm"
#include "vue.h"
//---------------------------------------------------------------------------
TfrmDimensionsZone *frmDimensionsZone = nullptr; // portage : =NULL explicite
//---------------------------------------------------------------------------
bool DefinitDimensionsZone(int &NbX, int &NbY, bool &GenereReseau)
{
    int mrRep = mrCancel;
    frmDimensionsZone = new TfrmDimensionsZone(Application);
    if (frmDimensionsZone)
    {
        { // portage : try/__finally -> garde RAII (Finalement)
            const Finalement f([&] {
                delete frmDimensionsZone;
                frmDimensionsZone = nullptr;
            });
            frmDimensionsZone->NbX = NbX;
            frmDimensionsZone->NbY = NbY;
            mrRep = frmDimensionsZone->ShowModal();
            if (mrRep == mrOk)
            {
                NbX = frmDimensionsZone->NbX;
                NbY = frmDimensionsZone->NbY;
                if (frmDimensionsZone->CheckBoxAfficherParametres->Checked)
                { // v4.1
                    frmSimulation->ActionParametres->Execute();
                }
                GenereReseau = frmDimensionsZone->CheckBoxGenererReseau->Checked; // v4.2
            }
        }
        return (mrRep == mrOk);
    }

    return false;
}
//---------------------------------------------------------------------------
TfrmDimensionsZone::TfrmDimensionsZone(TComponent *Owner) : TForm(Owner)
{
    ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
}
//---------------------------------------------------------------------------
void TfrmDimensionsZone::FormShow(TObject * /*Sender*/)
{
    CSpinEditNbX->Value = NbX;
    CSpinEditNbY->Value = NbY;
    // Autorise les modifications
    CSpinEditNbX->Tag = 0;
}
//---------------------------------------------------------------------------
void TfrmDimensionsZone::ButtonCreerClick(TObject * /*Sender*/)
{
    NbX = CSpinEditNbX->Value;
    NbY = CSpinEditNbY->Value;
    if ((NbX > NBXMAX) || (NbY > NBYMAX))
    { // v6.0.3 : aucune limite dans l'original (mémoire épuisée, plantage)
        AfficheMessage(Format("Vous devez spécifier une zone d'au plus %dx%d.", TableauDeConstantes(NBXMAX, NBYMAX)),
                       "Zone trop grande", tmAvertissement, 0, 0);
    }
    else if ((NbX >= NBXMIN) && (NbY >= NBYMIN))
    {
        ModalResult = mrOk;
    }
    else
    {
        AfficheMessage(Format("Vous devez spécifier une zone d'au moins %dx%d.", TableauDeConstantes(NBXMIN, NBYMIN)),
                       "Zone insuffisante", tmAvertissement, 0, 0);
    }
}
//---------------------------------------------------------------------------
void TfrmDimensionsZone::ButtonAideClick(TObject * /*Sender*/)
{
    AfficheAideContextuelle(this); // v3.8
}
//---------------------------------------------------------------------------

void TfrmDimensionsZone::FormKeyPress(TObject * /*Sender*/, char &Key)
{
    if (CSpinEditNbX->Focused() || CSpinEditNbY->Focused())
    {
        switch (Key)
        {
        case '\r':
            ButtonCreer->Click();
            break;
        case 27:
            ButtonAnnuler->Click();
            break;
        default:
            return;
        }
    }
}
//---------------------------------------------------------------------------

void TfrmDimensionsZone::FormKeyDown(TObject * /*Sender*/, WORD &Key, TShiftState /*Shift*/)
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
    Connecte(Composant->Evenement, this, &TfrmDimensionsZone::Gestionnaire);
// NOLINTEND(bugprone-macro-parentheses)
void TfrmDimensionsZone::ChargeFiche()
{
    Ui::frmDimensionsZone().setupUi(this);
    LieurComposants Lieur(this, "f_dimzone", Composants);
    COMPOSANTS_frmDimensionsZone(LIE_COMPOSANT) FinChargement(Lieur);
    EVENEMENTS_frmDimensionsZone(CONNECTE_EVENEMENT) if (OnCreate)
    {
        OnCreate(this);
    }
}
//---------------------------------------------------------------------------
