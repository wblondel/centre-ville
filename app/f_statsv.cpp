//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "base.h"
#include "simul.h"
#include "f_statsv.h"

#include <algorithm>
#include "f_simul.h"
//---------------------------------------------------------------------------
#include "ui_f_statsv.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_statsv_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfrmStatsVehicules *frmStatsVehicules;
//---------------------------------------------------------------------------
const AnsiString asTitre = "Statistique des véhicules d'une étape à l'autre [Tour %d]";
//---------------------------------------------------------------------------
TfrmStatsVehicules::TfrmStatsVehicules(TComponent* Owner)
    : TForm(Owner)
{ ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
}
//---------------------------------------------------------------------------
void TfrmStatsVehicules::FormShow(TObject * /*Sender*/)
{
 int i = 0;
 int const ne=cv->NbParkings;
 frmSimulation->ActionStatsParkings->Checked=true;
 frmStatsVehicules->Caption=Format(asTitre, ARRAYOFCONST(((int)cv->TourCrt)));
 StringGridCoordParkings->RowCount=1+ne;
 StringGridStatsVehicules->Cells[0][0]="A\\D";
 StringGridStatsVehicules->RowCount=1+ne;
 StringGridStatsVehicules->ColCount=1+ne;
 StringGridCoordParkings->Cells[0][0]="N°";
 StringGridCoordParkings->Cells[1][0]="Position";
 for(i=0; i<ne; i++)
  {
   StringGridCoordParkings->Cells[0][1+i]=Format("%3d", ARRAYOFCONST((1+i)));
   StringGridStatsVehicules->Cells[0][1+i]=Format("%3d", ARRAYOFCONST((1+i)));
   StringGridStatsVehicules->Cells[1+i][0]=Format("%3d", ARRAYOFCONST((1+i)));
   StringGridCoordParkings->Cells[1][1+i]=Format("%d,%d",
                                               ARRAYOFCONST((cv->Parking[i].x,
                                                             cv->Parking[i].y)));
  }
 RafraichitInfos(); // v6.0.2 : l'original laissait la grille vide jusqu'au tour suivant
}
//---------------------------------------------------------------------------
void TfrmStatsVehicules::FormHide(TObject * /*Sender*/)
{
 frmSimulation->ActionStatsParkings->Checked=false;
}
//---------------------------------------------------------------------------
void TfrmStatsVehicules::ButtonFermerClick(TObject * /*Sender*/)
{
 Hide();
}
//---------------------------------------------------------------------------
void TfrmStatsVehicules::RafraichitInfos() const
{
 int i = 0;
 int ed = 0;
 int ea = 0;
 int const n=StringGridStatsVehicules->ColCount-1;
 int Record = 0;
 int Minimum = 0;
 int Maximum = 0;
 vehicule *v = nullptr;
 frmStatsVehicules->Caption=Format(asTitre, ARRAYOFCONST(((int)cv->TourCrt)));
 for(i=0; i<n*n; i++) {
  if ((i%n)!=(i/n)) {
   StringGridStatsVehicules->Cells[1+(i%n)][1+(i/n)]="   0";
}
}
 switch(TabControl->TabIndex)
  {
   case 0: for(ed=1; ed<=n; ed++) {
            for(ea=1; ea<=n; ea++) {
             if (ed!=ea)
              {
               Maximum=0; Minimum=32768;
               for(i=0; i<cv->NbVehicules; i++)
                {
                 v=&cv->Vehicule[i];
                 if ((v->NumParkingDepart==ed)&&
                     (v->NumParkingArrivee==ea)&&
                     (!v->Defini())&&
                     (v->NbCasesParcourues != 0)) // Vehicule venant de l'étape ed et arrivé à l'étape ea
                  { // On lit les infos et on regarde le record en cours
                   Maximum = std::max(v->NbCasesParcourues, Maximum);
                   Minimum = std::min(v->NbCasesParcourues, Minimum);
                    Record=StrToInt(AnsiString(StringGridStatsVehicules->Cells[ed][ea]));
                   if (Maximum-Minimum>Record) { // Si record battu on écrit
                    StringGridStatsVehicules->Cells[ed][ea]=Format("%4d", ARRAYOFCONST((Maximum-Minimum)));
}
                  }
                }
              }
}
}
           break;
   case 1: for(ed=1; ed<=n; ed++) {
            for(ea=1; ea<=n; ea++) {
             if (ed!=ea)
              {
               Maximum=0; Minimum=32768;
               for(i=0; i<cv->NbVehicules; i++)
                {
                 v=&cv->Vehicule[i];
                 if ((v->NumParkingDepart==ed)&&
                     (v->NumParkingArrivee==ea)&&
                     (!v->Defini())&&
                     (v->TourDrnDepl != 0)) // Vehicule venant de l'étape ed et arrivé à l'étape ea
                  { // On lit les infos et on regarde le record en cours
                   Maximum = std::max(v->TourDrnDepl, Maximum);
                   Minimum = std::min(v->TourDrnDepl, Minimum);
                    Record=StrToInt(AnsiString(StringGridStatsVehicules->Cells[ed][ea]));
                   if (Maximum-Minimum>Record) { // Si record battu on écrit
                    StringGridStatsVehicules->Cells[ed][ea]=Format("%4d", ARRAYOFCONST((Maximum-Minimum)));
}
                  }
                }
              }
}
}
           break;
  }
 StringGridStatsVehicules->Repaint();
}
//---------------------------------------------------------------------------
void TfrmStatsVehicules::TabControlChange(TObject * /*Sender*/)
{
 RafraichitInfos();
}
//---------------------------------------------------------------------------
void TfrmStatsVehicules::ButtonAideClick(TObject * /*Sender*/)
{ // v3.0
 AfficheAideContextuelle(this); // v3.8
}
//---------------------------------------------------------------------------

void TfrmStatsVehicules::FormKeyDown(TObject * /*Sender*/, WORD &Key,
      TShiftState  /*Shift*/)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112) {
   AfficheAideContextuelle(this);
}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmStatsVehicules::Gestionnaire);
void TfrmStatsVehicules::ChargeFiche()
 {
  Ui::frmStatsVehicules().setupUi(this);
  LieurComposants Lieur(this, "f_statsv", Composants);
  COMPOSANTS_frmStatsVehicules(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmStatsVehicules(CONNECTE_EVENEMENT)
  if (OnCreate) { OnCreate(this);
}
 }
//---------------------------------------------------------------------------
