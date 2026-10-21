//---------------------------------------------------------------------------
// f_rech.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_rech.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_rech.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_rech.h"
#include "ui_f_rech.h"
#include "f_rech_composants.h"
//---------------------------------------------------------------------------
TfrmRecherche *frmRecherche;
//---------------------------------------------------------------------------
TfrmRecherche::TfrmRecherche(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmRecherche::ChargeFiche()
 {
  Ui::frmRecherche().setupUi(this);
  LieurComposants Lieur(this, "f_rech", Composants);
  COMPOSANTS_frmRecherche(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditXChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditYChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditParkingChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditFeuChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditVehiculeChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditPietonChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditArretBusChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditBusChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditArretTramChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditTramChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditTaxiChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditPlaceTaxiChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditFileTaxiChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditPlaceParkChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditFileParkChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditVehlibChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
void TfrmRecherche::CSpinEditPlaceVehlibChange(TObject *Sender) { (void)Sender; }
//---------------------------------------------------------------------------
