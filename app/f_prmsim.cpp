//---------------------------------------------------------------------------
// f_prmsim.cpp - Portage de Centre-Ville : FICHE NON PORTÉE (sprint 2)
//
// La fiche est créée comme dans l'original et affichée telle que générée par
// tools/dfm2ui.py depuis src/f_prmsim.dfm, mais sans son code : aucun
// gestionnaire d'événement n'est connecté, et les méthodes appelées par les
// autres fiches ou par le moteur ne font rien. Le portage de src/f_prmsim.cpp
// est prévu en phase 6 de PLAN_PORTAGE.md.
//---------------------------------------------------------------------------
#include "f_prmsim.h"
#include "f_simul.h"
#include "ui_f_prmsim.h"
#include "f_prmsim_composants.h"
//---------------------------------------------------------------------------
TfrmParamSimul *frmParamSimul=NULL;
//---------------------------------------------------------------------------
TfrmParamSimul::TfrmParamSimul(TComponent* Owner) : TForm(Owner)
 {
  ChargeFiche();
  
 }
//---------------------------------------------------------------------------
void TfrmParamSimul::ChargeFiche()
 {
  Ui::frmParamSimul().setupUi(this);
  LieurComposants Lieur(this, "f_prmsim", Composants);
  COMPOSANTS_frmParamSimul(LIE_COMPOSANT)
  FinChargement(Lieur);
 }
//---------------------------------------------------------------------------
void Parametres()
 {
  if ((frmParamSimul=new TfrmParamSimul(frmSimulation)))
   {
	frmParamSimul->ShowModal();
	delete frmParamSimul;
	frmParamSimul=NULL;
   }
 }
//---------------------------------------------------------------------------
