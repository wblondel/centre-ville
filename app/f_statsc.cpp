//---------------------------------------------------------------------------
#include "vcl/fiche.h" // portage : remplace <vcl.h>
//#pragma hdrstop // portage
//---------------------------------------------------------------------------
#include "base.h"
#include "simul.h"
#include "f_statsc.h"
//---------------------------------------------------------------------------
#include "ui_f_statsc.h" // portage : #pragma package -> fiche générée par tools/dfm2ui.py
#include "f_statsc_composants.h" // portage : #pragma resource "*.dfm"
//---------------------------------------------------------------------------
#define SURROUTE  0
#define SURVOIE   1
//---------------------------------------------------------------------------
TfrmStatsComposition *frmStatsComposition=nullptr; // portage : =NULL explicite
//---------------------------------------------------------------------------
const AnsiString asFrmPC="###.##"; // v5.4, le format de FormatFloat a changé : "###.##" au lieu de "%6.2f"
//---------------------------------------------------------------------------
void Composition()
 {
  if ((frmStatsComposition=new TfrmStatsComposition(Application)))
   { // portage : try/__finally -> garde RAII (Finalement)
    Finalement f([&]
    {
     frmStatsComposition->Release();
     frmStatsComposition=nullptr;
    });
     frmStatsComposition->ShowModal();
   }
 }
//---------------------------------------------------------------------------
TfrmStatsComposition::TfrmStatsComposition(TComponent* Owner)
    : TForm(Owner)
{ ChargeFiche(); // portage : chargement du .dfm, puis OnCreate
}
//---------------------------------------------------------------------------
void TfrmStatsComposition::FormShow(TObject *Sender)
{
 voie *vv;
 int i;
 int x;
 int y;
 int Total=0;
 int Routes=0;
 int VoiesTram=0;
 int // v3.5
     Priorites[2]={0,0};
 int Parkings=0;
 int CasesArretsBus=0;
 int // v3.0
     ArretsBus=0;
 int // v3.0
     CasesArretsTram=0;
 int // v3.5
     ArretsTram=0;
 int // v3.5
     PlacesTaxi=0;
 int // v3.6
     PlacesPark=0;
 int // v5.0
     Feux[2]={0,0};
 int //v3.5
     Clp[2]={0,0};
 int //v3.5
     Stops[2]={0,0};
 int //v3.5
     Dir=0;
 int Nord=0;
 int Est=0;
 int Sud=0;
 int Ouest=0;
 for(i=0; i<cv->NbArretsBus; i++) { // v3.0
  ArretsBus+=static_cast<int>(cv->ArretBus[i].Defini());
}
 for(i=0; i<cv->NbArretsTram; i++) { // v3.5
  ArretsTram+=static_cast<int>(cv->ArretTram[i].Defini());
}
 for(i=0; i<cv->NbPlacesTaxi; i++) { // v3.6
  PlacesTaxi+=static_cast<int>(cv->PlaceTaxi[i].Definie());
}
 for(i=0; i<cv->NbPlacesPark; i++) { // v5.0
  PlacesPark+=static_cast<int>(cv->PlacePark[i].Definie());
}
 for(x=0; x<cv->NbX; x++) {
  for(y=0; y<cv->NbY; y++)
   {
    vv=&(cv->v[x][y]);
    Total++;
    Parkings+=static_cast<int>(vv->NumParking>0);
    CasesArretsBus+=static_cast<int>(vv->NumArretBus>0); // v3.0
    if (vv->DirPoss != 0) // Route
     {
      Routes++;
      Dir+=NbDir[vv->DirPoss];
      Nord+=static_cast<int>((vv->DirPoss&N)>0);
      Est+=static_cast<int>((vv->DirPoss&E)>0);
      Sud+=static_cast<int>((vv->DirPoss&S)>0);
      Ouest+=static_cast<int>((vv->DirPoss&O)>0);
      for(i=0; i<NBDIR; i++) {
       if ((vv->DirPoss&PossDir[static_cast<direction>(i+1)]) != 0) { // v3.5
        switch(vv->Priorite[i])
         {
          case feux:             Priorites[SURROUTE]++; Feux[SURROUTE]++; break;
          case cedez_le_passage: Priorites[SURROUTE]++; Clp[SURROUTE]++; break;
          case stop:             Priorites[SURROUTE]++; Stops[SURROUTE]++; break;
          default: break;
         }
}
}
     }
    if (vv->SensVoieTram != 0) // v3.5
     {
      VoiesTram++;
      CasesArretsTram+=static_cast<int>(vv->NumArretTram>0);
      for(i=0; i<NBDIR; i++) {
       if (vv->EstDirPossSensVoieTram(indefinie, static_cast<direction>(i+1))) {
        switch(vv->Priorite[i])
         {
          case feux:             Priorites[SURVOIE]++; Feux[SURVOIE]++; break;
          case cedez_le_passage: Priorites[SURVOIE]++; Clp[SURVOIE]++; break;
          case stop:             Priorites[SURVOIE]++; Stops[SURVOIE]++; break;
          default: break;
         }
}
}
     }
   }
}
 LabelTotal->Caption=IntToStr(Total);
 LabelRoutes->Caption=IntToStr(Routes);
 // v1.8.4 : FormatFloat à la place de Format dans toute la suite
 LabelRoutesPC->Caption=FormatFloat(asFrmPC, 100.0*Routes/Total);
 if (Routes != 0)
  {
   LabelPrioritesR->Caption=IntToStr(Priorites[SURROUTE]);
   LabelPrioritesRPC->Caption=FormatFloat(asFrmPC, 100.0*Priorites[SURROUTE]/Routes);
   if (Priorites[SURROUTE] != 0)
    {
     LabelFeuxR->Caption=IntToStr(Feux[SURROUTE]);
	 LabelFeuxRPC->Caption=FormatFloat(asFrmPC, 100.0*Feux[SURROUTE]/Priorites[SURROUTE]);
	 LabelFeuxRMax->Caption=IntToStr(cv->NbFeux);
	 LabelFeuxRReste->Caption=IntToStr(cv->NbFeux-Feux[SURVOIE]-Feux[SURROUTE]);
	 LabelClpR->Caption=IntToStr(Clp[SURROUTE]);
	 LabelClpRPC->Caption=FormatFloat(asFrmPC, 100.0*Clp[SURROUTE]/Priorites[SURROUTE]);
	 LabelStopsR->Caption=IntToStr(Stops[SURROUTE]);  // v3.8 : oubli de [SURROUTE] !
	 LabelStopsRPC->Caption=FormatFloat(asFrmPC, 100.0*Stops[SURROUTE]/Priorites[SURROUTE]);
    }
   LabelParkings->Caption=IntToStr(Parkings);
   LabelParkingsPC->Caption=FormatFloat(asFrmPC, 100.0*Parkings/Routes);
   LabelParkingsMax->Caption=IntToStr(cv->NbParkings);
   LabelParkingsReste->Caption=IntToStr(cv->NbParkings-Parkings);
   LabelDir->Caption=IntToStr(Dir);
   LabelDirPC->Caption=FormatFloat(asFrmPC, 100.0*Dir/Routes);
   if (Dir != 0)
	{
	 LabelNord->Caption=IntToStr(Nord);
	 LabelNordPC->Caption=FormatFloat(asFrmPC, 100.0*Nord/Dir);
     LabelEst->Caption=IntToStr(Est);
	 LabelEstPC->Caption=FormatFloat(asFrmPC, 100.0*Est/Dir);
	 LabelSud->Caption=IntToStr(Sud);
	 LabelSudPC->Caption=FormatFloat(asFrmPC, 100.0*Sud/Dir);
	 LabelOuest->Caption=IntToStr(Ouest);
	 LabelOuestPC->Caption=FormatFloat(asFrmPC, 100.0*Ouest/Dir);
	}
  }
 if (ArretsBus != 0) // v3.0
  {
   LabelArretsBus->Caption=IntToStr(ArretsBus);
   if (Routes != 0) { LabelArretsBusPC->Caption=FormatFloat(asFrmPC, 100.0*CasesArretsBus/Routes);
}
   LabelArretsBusMax->Caption=IntToStr(cv->NbArretsBus);
   LabelArretsBusReste->Caption=IntToStr(cv->NbArretsBus-ArretsBus);
  }
 // v3.5
 LabelVoiesTram->Caption=IntToStr(VoiesTram);
 LabelVoiesTramPC->Caption=FormatFloat(asFrmPC, 100.0*VoiesTram/Total);
 if (ArretsTram != 0)
  {
   LabelArretsTram->Caption=IntToStr(ArretsTram);
   if (VoiesTram != 0) { LabelArretsTramPC->Caption=FormatFloat(asFrmPC, 100.0*CasesArretsTram/VoiesTram);
}
   LabelArretsTramMax->Caption=IntToStr(cv->NbArretsTram);
   LabelArretsTramReste->Caption=IntToStr(cv->NbArretsTram-ArretsTram);
  }

 LabelPlacesTaxi->Caption=IntToStr(PlacesTaxi);
 if (Routes != 0) { LabelPlacesTaxiPC->Caption=FormatFloat(asFrmPC, 100.0*PlacesTaxi/Routes);
}
 LabelPlacesTaxiMax->Caption=IntToStr(cv->NbPlacesTaxi);
 LabelPlacesTaxiReste->Caption=IntToStr(cv->NbPlacesTaxi-PlacesTaxi);
 // v5.0
 LabelPlacesPark->Caption=IntToStr(PlacesPark);
 if (Routes != 0) { LabelPlacesParkPC->Caption=FormatFloat(asFrmPC, 100.0*PlacesPark/Routes);
}
 LabelPlacesParkMax->Caption=IntToStr(cv->NbPlacesPark);
 LabelPlacesParkReste->Caption=IntToStr(cv->NbPlacesPark-PlacesPark);

 LabelPrioritesV->Caption=IntToStr(Priorites[SURVOIE]);
 if (VoiesTram != 0) { LabelPrioritesVPC->Caption=FormatFloat(asFrmPC, 100.0*Priorites[SURVOIE]/VoiesTram);
}
 if (Priorites[SURVOIE] != 0)
  {
   LabelFeuxV->Caption=IntToStr(Feux[SURVOIE]);
   LabelFeuxVPC->Caption=FormatFloat(asFrmPC, 100.0*Feux[SURVOIE]/Priorites[SURVOIE]);
   LabelFeuxVMax->Caption=IntToStr(cv->NbFeux);
   LabelFeuxVReste->Caption=IntToStr(cv->NbFeux-Feux[SURVOIE]-Feux[SURROUTE]);
   LabelClpV->Caption=IntToStr(Clp[SURVOIE]);
   LabelClpVPC->Caption=FormatFloat(asFrmPC, 100.0*Clp[SURVOIE]/Priorites[SURVOIE]);
   LabelStopsV->Caption=IntToStr(Stops[SURVOIE]); // v3.8 : oubli de [SURVOIE] !
   LabelStopsVPC->Caption=FormatFloat(asFrmPC, 100.0*Stops[SURVOIE]/Priorites[SURVOIE]);
  }
}
//---------------------------------------------------------------------------
void TfrmStatsComposition::ButtonAideClick(TObject *Sender)
{ // v3.0
 AfficheAideContextuelle(this); // v3.8
}
//---------------------------------------------------------------------------



void TfrmStatsComposition::FormKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{// v5.3 (nécessite TForm:KeyPreview à true)
 if (Key==112) {
   AfficheAideContextuelle(this);
}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// portage : chargement du .dfm (fiche générée par tools/dfm2ui.py) et gestionnaires
//---------------------------------------------------------------------------
#define CONNECTE_EVENEMENT(Composant, Evenement, Gestionnaire) Connecte(Composant->Evenement, this, &TfrmStatsComposition::Gestionnaire);
void TfrmStatsComposition::ChargeFiche()
 {
  Ui::frmStatsComposition().setupUi(this);
  LieurComposants Lieur(this, "f_statsc", Composants);
  COMPOSANTS_frmStatsComposition(LIE_COMPOSANT)
  FinChargement(Lieur);
  EVENEMENTS_frmStatsComposition(CONNECTE_EVENEMENT)
  if (OnCreate) { OnCreate(this);
}
 }
//---------------------------------------------------------------------------
